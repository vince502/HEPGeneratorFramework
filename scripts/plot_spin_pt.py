import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
import sys

def spin_dist(cos_theta, norm, rho00):
    return norm * ( (1 - rho00) + (3 * rho00 - 1) * cos_theta**2 )

def fit_rho00(cos_thetas):
    if len(cos_thetas) < 50: # Need enough data to fit
        return None, None
    counts, bin_edges = np.histogram(cos_thetas, bins=20, range=(-1, 1))
    bin_centers = (bin_edges[:-1] + bin_edges[1:]) / 2
    yerr = np.sqrt(counts)
    yerr[yerr == 0] = 1
    try:
        popt, pcov = curve_fit(spin_dist, bin_centers, counts, p0=[max(counts), 0.33], sigma=yerr)
        return popt[1], np.sqrt(pcov[1, 1])
    except:
        return None, None

def main():
    input_file = "cos_theta_pt_bins.txt"
    if len(sys.argv) > 1:
        input_file = sys.argv[1]
    
    try:
        # Load: Type (0/1), pT, cosTheta
        data = np.loadtxt(input_file)
    except Exception as e:
        print(f"Error: {e}")
        return

    pt_bins = [3, 5, 7, 10, 15, 20, 30]
    
    results = {0: [], 1: []} # 0=prompt, 1=non-prompt
    
    for category in [0, 1]:
        cat_data = data[data[:, 0] == category]
        for i in range(len(pt_bins) - 1):
            pt_min, pt_max = pt_bins[i], pt_bins[i+1]
            bin_data = cat_data[(cat_data[:, 1] >= pt_min) & (cat_data[:, 1] < pt_max)]
            
            rho, err = fit_rho00(bin_data[:, 2])
            if rho is not None:
                results[category].append({
                    'pt_mid': (pt_min + pt_max) / 2,
                    'pt_width': (pt_max - pt_min) / 2,
                    'rho': rho,
                    'err': err
                })

    # Plotting
    plt.figure(figsize=(10, 7))
    
    labels = {0: 'Prompt $D^0$ (from $D^*$)', 1: 'Non-prompt $D^0$ (from $B$)'}
    colors = {0: 'blue', 1: 'red'}
    markers = {0: 'o', 1: 's'}
    
    for category in [0, 1]:
        if not results[category]: continue
        
        pts = [r['pt_mid'] for r in results[category]]
        pt_errs = [r['pt_width'] for r in results[category]]
        rhos = [r['rho'] for r in results[category]]
        errs = [r['err'] for r in results[category]]
        
        plt.errorbar(pts, rhos, xerr=pt_errs, yerr=errs, fmt=markers[category], 
                     color=colors[category], label=labels[category], capsize=3)

    plt.axhline(1/3, color='gray', linestyle='--', label='Unpolarized (1/3)')
    plt.xlabel('$p_T$ [GeV/c]')
    plt.ylabel(r'$\rho_{00}$')
    plt.title(r'Spin Alignment $\rho_{00}$ vs $p_T$ in Pb-Pb (5.02 TeV)')
    plt.ylim(0.1, 0.6)
    plt.xlim(3, 30)
    plt.legend()
    plt.grid(True, alpha=0.3)
    
    plt.savefig("rho00_vs_pt.png", dpi=150)
    print("Plot saved to rho00_vs_pt.png")

if __name__ == "__main__":
    main()
