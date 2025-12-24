import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
import sys

def spin_dist(cos_theta, norm, rho00):
    """Normalized spin distribution for vector -> scalar scalar:
    W(cos_theta) = 3/4 * [ (1-rho00) + (3*rho00-1)*cos_theta^2 ]
    We use a free normalization constant C.
    """
    return norm * ( (1 - rho00) + (3 * rho00 - 1) * cos_theta**2 )

def main():
    input_file = "cos_theta_counts.txt"
    if len(sys.argv) > 1:
        input_file = sys.argv[1]
    
    try:
        data = np.loadtxt(input_file)
    except Exception as e:
        print(f"Error loading data: {e}")
        return

    # Histogram
    counts, bin_edges = np.histogram(data, bins=40, range=(-1, 1))
    bin_centers = (bin_edges[:-1] + bin_edges[1:]) / 2
    
    # Uncertainty
    yerr = np.sqrt(counts)
    yerr[yerr == 0] = 1 # avoid zero error
    
    # Fit
    popt, pcov = curve_fit(spin_dist, bin_centers, counts, p0=[max(counts), 0.33], sigma=yerr)
    norm_fit, rho00_fit = popt
    rho00_err = np.sqrt(pcov[1, 1])

    # Plot
    plt.figure(figsize=(8, 6))
    plt.errorbar(bin_centers, counts, yerr=yerr, fmt='ko', label='Data (D* -> D0 pi)')
    
    x_fit = np.linspace(-1, 1, 100)
    y_fit = spin_dist(x_fit, *popt)
    plt.plot(x_fit, y_fit, 'r-', label=f'Fit: rho00 = {rho00_fit:.3f} +/- {rho00_err:.3f}')
    
    plt.xlabel(r'$\cos \theta^*$ (Helicity Angle)')
    plt.ylabel('Counts')
    plt.title(r'Spin Alignment Study for Prompt $D^0$ (from $D^*$)')
    plt.legend()
    plt.grid(True, alpha=0.3)
    
    plot_file = "rho00_plot.png"
    plt.savefig(plot_file, dpi=150)
    print(f"Plot saved to {plot_file}")
    print(f"Measured rho00: {rho00_fit:.4f} +/- {rho00_err:.4f}")

if __name__ == "__main__":
    main()
