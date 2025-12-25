import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
import sys

def spin_dist(cos_theta, norm, rho00):
    """Spin density distribution: W(θ) ∝ (1-ρ00) + (3ρ00-1)cos²θ"""
    return norm * ( (1 - rho00) + (3 * rho00 - 1) * cos_theta**2 )

def fit_rho00(cos_thetas):
    if len(cos_thetas) < 50:
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

def calculate_v2(cos2_delta_phi):
    """v2 = <cos(2ΔΦ)>"""
    if len(cos2_delta_phi) < 50:
        return None, None
    v2 = np.mean(cos2_delta_phi)
    # Statistical error: σ/√N
    err = np.std(cos2_delta_phi) / np.sqrt(len(cos2_delta_phi))
    return v2, err

def main():
    input_file = "cos_theta_pt_bins.txt"
    if len(sys.argv) > 1:
        input_file = sys.argv[1]
    
    try:
        # Load: type pT rapidity cosTheta cos2DeltaPhi
        data = np.loadtxt(input_file, usecols=(0, 1, 2, 3, 4))
    except Exception as e:
        print(f"Error loading data: {e}")
        print("Expected columns: type pT rapidity cosTheta cos2DeltaPhi")
        return

    pt_bins = [3, 5, 7, 10, 15, 20, 30]
    
    # Results storage
    results_rho00 = {0: [], 1: []}  # 0=prompt, 1=non-prompt
    results_v2 = {0: [], 1: []}
    
    for category in [0, 1]:
        cat_data = data[data[:, 0] == category]
        for i in range(len(pt_bins) - 1):
            pt_min, pt_max = pt_bins[i], pt_bins[i+1]
            bin_data = cat_data[(cat_data[:, 1] >= pt_min) & (cat_data[:, 1] < pt_max)]
            
            if len(bin_data) == 0:
                continue
            
            # Spin alignment (ρ00)
            rho, rho_err = fit_rho00(bin_data[:, 3])
            if rho is not None:
                results_rho00[category].append({
                    'pt_mid': (pt_min + pt_max) / 2,
                    'pt_width': (pt_max - pt_min) / 2,
                    'val': rho,
                    'err': rho_err
                })
            
            # v2 flow
            v2, v2_err = calculate_v2(bin_data[:, 4])
            if v2 is not None:
                results_v2[category].append({
                    'pt_mid': (pt_min + pt_max) / 2,
                    'pt_width': (pt_max - pt_min) / 2,
                    'val': v2,
                    'err': v2_err
                })

    # Create figure with two panels
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))
    
    labels = {0: 'Prompt $D^0$', 1: 'Non-prompt $D^0$'}
    colors = {0: 'blue', 1: 'red'}
    markers = {0: 'o', 1: 's'}
    
    # === Left panel: Spin Alignment (ρ00) ===
    for category in [0, 1]:
        if not results_rho00[category]: continue
        pts = [r['pt_mid'] for r in results_rho00[category]]
        pt_errs = [r['pt_width'] for r in results_rho00[category]]
        vals = [r['val'] for r in results_rho00[category]]
        errs = [r['err'] for r in results_rho00[category]]
        ax1.errorbar(pts, vals, xerr=pt_errs, yerr=errs, fmt=markers[category], 
                     color=colors[category], label=labels[category], capsize=3)
    
    ax1.axhline(1/3, color='gray', linestyle='--', label='Unpolarized (1/3)')
    ax1.set_xlabel('$p_T$ [GeV/c]')
    ax1.set_ylabel(r'$\rho_{00}$')
    ax1.set_title(r'Spin Alignment $\rho_{00}$ vs $p_T$')
    ax1.set_ylim(0.1, 0.6)
    ax1.set_xlim(3, 30)
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    
    # === Right panel: v2 Flow ===
    for category in [0, 1]:
        if not results_v2[category]: continue
        pts = [r['pt_mid'] for r in results_v2[category]]
        pt_errs = [r['pt_width'] for r in results_v2[category]]
        vals = [r['val'] for r in results_v2[category]]
        errs = [r['err'] for r in results_v2[category]]
        ax2.errorbar(pts, vals, xerr=pt_errs, yerr=errs, fmt=markers[category], 
                     color=colors[category], label=labels[category], capsize=3)
    
    ax2.axhline(0, color='gray', linestyle='--', alpha=0.5)
    ax2.set_xlabel('$p_T$ [GeV/c]')
    ax2.set_ylabel(r'$v_2 = \langle \cos(2\Delta\phi) \rangle$')
    ax2.set_title(r'Elliptic Flow $v_2$ vs $p_T$')
    ax2.set_ylim(-0.1, 0.3)
    ax2.set_xlim(3, 30)
    ax2.legend()
    ax2.grid(True, alpha=0.3)
    
    plt.suptitle('D$^0$ Analysis in Pb-Pb @ 5.02 TeV (Angantyr)', fontsize=14)
    plt.tight_layout()
    
    plt.savefig("d0_spin_and_v2.png", dpi=150)
    print("Plot saved to d0_spin_and_v2.png")
    
    # Print summary
    print("\n=== Summary ===")
    for category in [0, 1]:
        name = "Prompt" if category == 0 else "Non-prompt"
        if results_rho00[category]:
            print(f"\n{name} D0:")
            for r in results_rho00[category]:
                print(f"  pT={r['pt_mid']:.0f} GeV: ρ00 = {r['val']:.3f} ± {r['err']:.3f}")
        if results_v2[category]:
            for r in results_v2[category]:
                print(f"  pT={r['pt_mid']:.0f} GeV: v2 = {r['val']:.4f} ± {r['err']:.4f}")

if __name__ == "__main__":
    main()
