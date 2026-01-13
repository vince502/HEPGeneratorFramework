#!/usr/bin/env python3
"""
pkl_to_root.py - Convert pickle histogram file to ROOT format using uproot

Usage:
    python3 pkl_to_root.py input.pkl [output.root]

Requires uproot to be installed (pip install uproot).
"""

import sys
import pickle
import numpy as np

try:
    import uproot
except ImportError:
    print("Error: uproot not found. Install with: pip install uproot")
    sys.exit(1)


def dict_to_uproot_hist(data):
    """Convert histogram dictionary to uproot-writable format."""
    if 'bins' not in data:
        return None
    
    bins = data['bins']
    if not bins:
        return None
    
    # Get bin edges
    edges = [bins[0]['xlow']]
    for b in bins:
        edges.append(b['xhigh'])
    
    edges_arr = np.array(edges, dtype=np.float64)
    
    # Get bin contents and variances
    contents = np.array([b['sumw'] for b in bins], dtype=np.float64)
    variances = np.array([b['sumw2'] for b in bins], dtype=np.float64)
    
    return {
        'edges': edges_arr,
        'contents': contents,
        'variances': variances,
        'entries': data.get('numEntries', sum(contents))
    }


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)
    
    input_pkl = sys.argv[1]
    output_root = sys.argv[2] if len(sys.argv) > 2 else input_pkl.replace('.pkl', '.root')
    
    print(f"Reading: {input_pkl}")
    with open(input_pkl, 'rb') as f:
        histos = pickle.load(f)
    
    print(f"Writing: {output_root}")
    
    # Prepare data for uproot
    to_write = {}
    
    for path, data in histos.items():
        if 'bins' not in data:
            continue
        
        hist_data = dict_to_uproot_hist(data)
        if hist_data is None:
            continue
        
        # Clean name for ROOT
        clean_name = path.replace('/', '_').lstrip('_').replace('[', '_').replace(']', '').replace('-', '_')
        
        # Create histogram using uproot's TH1 format
        # uproot uses numpy histograms with (data, edges) format
        edges = hist_data['edges']
        contents = hist_data['contents']
        variances = hist_data['variances']
        
        # Create a histogram object that uproot can write
        # We need to use uproot.writing to create proper TH1D
        to_write[clean_name] = uproot.writing.identify.to_TH1x(
            fName=clean_name,
            fTitle=data.get('title', clean_name),
            data=contents,
            fEntries=hist_data['entries'],
            fTsumw=np.sum(contents),
            fTsumw2=np.sum(variances),
            fTsumwx=np.sum(contents * (edges[:-1] + edges[1:]) / 2),
            fTsumwx2=np.sum(contents * ((edges[:-1] + edges[1:]) / 2)**2),
            fSumw2=variances,
            fXaxis=uproot.writing.identify.to_TAxis(
                fName="xaxis",
                fTitle="",
                fNbins=len(contents),
                fXmin=edges[0],
                fXmax=edges[-1],
                fXbins=edges if len(set(np.diff(edges))) > 1 else np.array([]),
            ),
        )
        print(f"  Prepared: {clean_name}")
    
    # Write to ROOT file
    with uproot.recreate(output_root) as f:
        for name, hist in to_write.items():
            f[name] = hist
    
    print(f"\nDone! Wrote {len(to_write)} histograms to {output_root}")


if __name__ == "__main__":
    main()
