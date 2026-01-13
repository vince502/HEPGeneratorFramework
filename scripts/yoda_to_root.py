#!/usr/bin/env python3
"""
yoda_to_root.py - Convert YODA histograms to ROOT and Python formats

Usage:
    python3 yoda_to_root.py input.yoda [output_basename]

Output:
    - output_basename.root  : ROOT file with TH1D histograms
    - output_basename.pkl   : Python pickle with histogram data
"""

import sys
import pickle
import numpy as np

try:
    import yoda
    YODA_VERSION = int(yoda.version().split('.')[0])
    print(f"YODA version: {yoda.version()} (major: {YODA_VERSION})")
except ImportError:
    print("Error: yoda module not found. Run inside the cmsana-rivet container.")
    sys.exit(1)

try:
    import ROOT
    HAS_ROOT = True
except ImportError:
    print("Warning: ROOT not available. Skipping ROOT output.")
    HAS_ROOT = False


def safe_get_error(obj):
    """Safely get error value from YODA 2.x objects."""
    try:
        # Try different error access methods
        if hasattr(obj, 'errAvg'):
            return obj.errAvg()
        if hasattr(obj, 'relErr'):
            val = safe_get_value(obj)
            return abs(val * obj.relErr()) if val != 0 else 0
        if hasattr(obj, 'err'):
            try:
                return obj.err()
            except:
                # err() might need a key argument
                try:
                    return obj.err('')
                except:
                    return 0
        if hasattr(obj, 'stdErr'):
            return obj.stdErr()
    except:
        pass
    return 0


def safe_get_value(obj):
    """Safely get value from YODA 2.x objects."""
    try:
        if hasattr(obj, 'val'):
            return obj.val()
        if hasattr(obj, 'sumW'):
            return obj.sumW()
        if hasattr(obj, 'height'):
            return obj.height()
    except:
        pass
    return 0


def get_bin_data_v2(h):
    """Extract bin data from YODA 2.x BinnedHisto1D or BinnedEstimate1D."""
    bins = []
    
    # Try different attribute names for YODA 2.x
    if hasattr(h, 'bins'):
        try:
            bin_iter = list(h.bins())
        except:
            bin_iter = []
    else:
        try:
            bin_iter = list(h)
        except:
            bin_iter = []
    
    for b in bin_iter:
        bin_data = {}
        
        # Get edges - YODA 2 uses xEdges or xMin/xMax
        try:
            if hasattr(b, 'xEdges'):
                edges = b.xEdges()
                bin_data['xlow'] = edges[0]
                bin_data['xhigh'] = edges[1]
            elif hasattr(b, 'xMin'):
                bin_data['xlow'] = b.xMin()
                bin_data['xhigh'] = b.xMax()
            elif hasattr(b, 'xMid'):
                # Estimate from mid and width
                mid = b.xMid()
                width = b.xWidth() if hasattr(b, 'xWidth') else 1.0
                bin_data['xlow'] = mid - width/2
                bin_data['xhigh'] = mid + width/2
            else:
                continue
        except Exception as e:
            continue
        
        # Get values
        val = safe_get_value(b)
        err = safe_get_error(b)
        
        bin_data['sumw'] = val
        bin_data['sumw2'] = err**2 if err else val**2 * 0.01  # fallback to 1% if no error
        bin_data['numentries'] = b.numEntries() if hasattr(b, 'numEntries') else (1 if val != 0 else 0)
        
        bins.append(bin_data)
    
    return bins


def yoda_histo_to_dict(h):
    """Convert a YODA histogram to a dictionary with bin info."""
    bins = get_bin_data_v2(h)
    
    if not bins:
        raise ValueError("No bins could be extracted")
    
    result = {
        'name': h.name() if hasattr(h, 'name') else str(h),
        'path': h.path() if hasattr(h, 'path') else '',
        'title': h.title() if hasattr(h, 'title') else '',
        'bins': bins,
        'underflow': {'sumw': 0, 'sumw2': 0},
        'overflow': {'sumw': 0, 'sumw2': 0},
    }
    
    # Compute integral and entries from bins
    result['integral'] = sum(b['sumw'] for b in bins)
    result['numEntries'] = sum(b['numentries'] for b in bins)
    
    return result


def yoda_histo_to_th1d(h, name):
    """Convert a YODA histogram to ROOT TH1D."""
    if not HAS_ROOT:
        return None
    
    bins = get_bin_data_v2(h)
    if not bins:
        return None
    
    # Get bin edges
    edges = [bins[0]['xlow']]
    for b in bins:
        edges.append(b['xhigh'])
    
    edges_arr = np.array(edges, dtype=np.float64)
    nbins = len(edges) - 1
    
    # Create TH1D with variable binning
    title = h.title() if hasattr(h, 'title') else name
    th1 = ROOT.TH1D(name, title, nbins, edges_arr)
    
    # Fill bin contents and errors
    for i, b in enumerate(bins):
        th1.SetBinContent(i + 1, b['sumw'])
        th1.SetBinError(i + 1, np.sqrt(b['sumw2']) if b['sumw2'] > 0 else 0)
    
    return th1


def process_counter(ao, path):
    """Process Counter/Estimate0D type."""
    val = safe_get_value(ao)
    err = safe_get_error(ao)
    
    return {
        'name': ao.name() if hasattr(ao, 'name') else path,
        'path': path,
        'type': 'Counter',
        'sumw': val,
        'sumw2': err**2 if err else 0,
        'numEntries': ao.numEntries() if hasattr(ao, 'numEntries') else 1
    }


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)
    
    input_yoda = sys.argv[1]
    output_base = sys.argv[2] if len(sys.argv) > 2 else input_yoda.replace('.yoda', '')
    
    print(f"Reading: {input_yoda}")
    aos = yoda.read(input_yoda)
    
    # Prepare output containers
    histos_dict = {}
    
    if HAS_ROOT:
        root_file = ROOT.TFile(f"{output_base}.root", "RECREATE")
        print(f"Writing ROOT file: {output_base}.root")
    
    # Process each analysis object
    for path, ao in aos.items():
        # Skip RAW histograms (internal Rivet bookkeeping)
        if '/RAW/' in path:
            print(f"  Skipping RAW histogram: {path}")
            continue
            
        # Clean name for ROOT (remove leading slashes and special chars)
        clean_name = path.replace('/', '_').lstrip('_').replace('[', '_').replace(']', '')
        
        # Get type name
        type_name = type(ao).__name__
        
        # Handle different histogram types
        if 'Histo' in type_name or 'Estimate1D' in type_name or 'BinnedEstimate' in type_name:
            try:
                print(f"  Converting {type_name}: {path}")
                histos_dict[path] = yoda_histo_to_dict(ao)
                
                if HAS_ROOT:
                    th1 = yoda_histo_to_th1d(ao, clean_name)
                    if th1:
                        th1.Write()
            except Exception as e:
                print(f"    Warning: Failed to convert {path}: {e}")
        
        elif 'Counter' in type_name or 'Estimate0D' in type_name:
            try:
                print(f"  Converting {type_name}: {path}")
                histos_dict[path] = process_counter(ao, path)
            except Exception as e:
                print(f"    Warning: Failed to convert {path}: {e}")
        
        else:
            print(f"  Skipping unsupported type: {path} ({type_name})")
    
    # Save ROOT file
    if HAS_ROOT:
        root_file.Close()
        print(f"ROOT file saved: {output_base}.root")
    
    # Save Python pickle
    pkl_path = f"{output_base}.pkl"
    with open(pkl_path, 'wb') as f:
        pickle.dump(histos_dict, f)
    print(f"Pickle file saved: {pkl_path}")
    
    # Also save as readable JSON-like text
    txt_path = f"{output_base}_summary.txt"
    with open(txt_path, 'w') as f:
        f.write(f"# YODA to ROOT/Python Conversion Summary\n")
        f.write(f"# Input: {input_yoda}\n")
        f.write(f"# Histograms: {len(histos_dict)}\n\n")
        for path, data in histos_dict.items():
            if 'bins' in data:
                f.write(f"{path}\n")
                f.write(f"  Entries: {data['numEntries']}, Integral: {data['integral']:.4f}\n")
                f.write(f"  Bins: {len(data['bins'])}\n\n")
            else:
                f.write(f"{path} (Counter)\n")
                f.write(f"  Value: {data['sumw']:.4f}\n\n")
    print(f"Summary saved: {txt_path}")
    
    print(f"\nDone! Converted {len(histos_dict)} histograms.")


if __name__ == "__main__":
    main()
