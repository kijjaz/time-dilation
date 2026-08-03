import os
import matplotlib.pyplot as plt
import matplotlib

matplotlib.rcParams['text.usetex'] = False
matplotlib.rcParams['font.sans-serif'] = 'DejaVu Sans'

def render_tex_paper_to_pdf(tex_filepath, pdf_filepath, title_text, sections):
    fig, ax = plt.subplots(figsize=(8.5, 11), dpi=300)
    ax.axis('off')

    # Background
    fig.patch.set_facecolor('#ffffff')

    y_cursor = 0.95

    # Document Header Title
    ax.text(0.08, y_cursor, title_text, fontsize=14, fontweight='bold', color='#0f172a', va='top')
    y_cursor -= 0.05

    ax.text(0.08, y_cursor, "Time Dilation DAW Technical Proof Series (v0.0.1)", fontsize=10, fontstyle='italic', color='#64748b', va='top')
    y_cursor -= 0.04

    # Horizontal Divider Line
    ax.plot([0.08, 0.92], [y_cursor, y_cursor], color='#cbd5e1', lw=1.5, transform=ax.transAxes)
    y_cursor -= 0.04

    for sec_title, body_paragraphs in sections:
        if y_cursor < 0.12:
            break

        # Section Heading
        ax.text(0.08, y_cursor, sec_title, fontsize=12, fontweight='bold', color='#1e293b', va='top')
        y_cursor -= 0.035

        for para in body_paragraphs:
            if y_cursor < 0.10:
                break
            if para.startswith("$$") or para.startswith("\\["):
                # Render LaTeX Math Equation
                eq_clean = para.strip("$[]\\")
                ax.text(0.50, y_cursor, f"${eq_clean}$", fontsize=11, color='#0284c7', ha='center', va='top')
                y_cursor -= 0.045
            else:
                # Body Text
                ax.text(0.08, y_cursor, para, fontsize=9.5, color='#334155', va='top', wrap=True)
                y_cursor -= 0.04

    plt.tight_layout()
    plt.savefig(pdf_filepath, format='pdf', bbox_inches='tight')
    plt.close(fig)
    print(f"Generated {pdf_filepath} successfully!")

if __name__ == "__main__":
    math_dir = "math"

    # 12. ARC Peak Limiter
    render_tex_paper_to_pdf(
        os.path.join(math_dir, "12_arc_peak_limiter_math.tex"),
        os.path.join(math_dir, "12_arc_peak_limiter_math.pdf"),
        "12. ARC Safety Peak Limiter & Dynamic Gain Smoothing Math",
        [
            ("1. Overview & Ceiling Threshold Math", [
                "In relativistic time-dilated audio synthesis, dynamic time warping and feedback cascades can produce amplitude spikes.",
                "To prevent digital clipping, out~ (OutNode) incorporates an ARC Safety Peak Limiter operating at -1.5 dBFS ceiling.",
                "\\[ C = 10^{\\frac{-1.5}{20}} \\approx 0.8413951 \\]",
                "For stereo audio blocks, the instantaneous peak magnitude p[s] and target gain g_target[s] are computed as:"
            ]),
            ("2. Attack & Adaptive ARC Release Dynamics", [
                "\\[ g_{\\text{target}}[s] = \\min\\left(1.0, \\frac{C}{\\max(|s_L[s]|, |s_R[s]|)}\\right) \\]",
                "Attack time constant = 0.05 ms (50 us) for ultra-fast transient suppression.",
                "Adaptive ARC Release timing dynamically switches based on peak intensity ratio R = p[s] / C:",
                "\\[ \\tau_{\\text{rel}}(R) = 10\\text{ ms for transient spikes } (R < 1.5), \\quad 300\\text{ ms for sustained peaks } (R \\geq 1.5) \\]",
                "Final output audio signals are scaled by g[s], guaranteeing zero hard clipping at peak ceiling."
            ])
        ]
    )

    # 13. Lorentz Time Signal Composition
    render_tex_paper_to_pdf(
        os.path.join(math_dir, "13_lorentz_time_signal_composition.tex"),
        os.path.join(math_dir, "13_lorentz_time_signal_composition.pdf"),
        "13. Lorentz Time Signal Composition & Auto-Scaling Math",
        [
            ("1. Lorentz Velocity Addition Theorem", [
                "Time signals in time.math~, time.scope, and time.xy process relativistic dilation vectors gamma.",
                "Combining two time sources with factors gamma_1 and gamma_2 follows Einstein velocity composition with speed-of-light c = 4.0:",
                "\\[ v_1 = \\gamma_1 - 1.0, \\quad v_2 = \\gamma_2 - 1.0, \\quad v_{\\text{comp}} = \\frac{v_1 + v_2}{1.0 + \\frac{v_1 \\cdot v_2}{c^2}} \\]",
                "\\[ \\gamma_{\\text{comp}} = 1.0 + v_{\\text{comp}} = 1.0 + \\frac{(\\gamma_1 - 1.0) + (\\gamma_2 - 1.0)}{1.0 + \\frac{(\\gamma_1 - 1.0)(\\gamma_2 - 1.0)}{c^2}} \\]"
            ]),
            ("2. Dynamic Auto-Scaling & Phase Orbit Radius", [
                "For continuous time telemetry in time.scope, max peak M = max |y[i]| auto-scales headroom dynamically:",
                "\\[ S[n] = S[n-1] + \\lambda \\cdot (\\max(1.0, M \\cdot 1.15) - S[n-1]) \\]",
                "For dual-time 2D Lissajous phase orbits in time.xy, max radial extent R_max = max sqrt(x^2 + y^2) auto-scales 2D bounds:",
                "\\[ R_{\\text{scale}}[n] = R_{\\text{scale}}[n-1] + \\lambda \\cdot (\\max(1.0, R_{\\text{max}} \\cdot 1.15) - R_{\\text{scale}}[n-1]) \\]"
            ])
        ]
    )

    # 01. Relativistic Time Dilation & Universal timeIn Architecture
    render_tex_paper_to_pdf(
        os.path.join(math_dir, "01_relativistic_time_dilation.tex"),
        os.path.join(math_dir, "01_relativistic_time_dilation.pdf"),
        "01. Relativistic Time Dilation & Universal timeIn Architecture",
        [
            ("1. Relativistic Coordinate Time & Dilation Rate", [
                "Coordinate time tau(t) is modulated by relativistic factor gamma(t) in [-16.0, 16.0]:",
                "\\[ d\\tau = \\gamma(t) \\cdot dt, \\quad \\tau(t) = \\int_0^t \\gamma(t') \\, dt' \\]",
                "Special relativity velocity time dilation follows Lorentz factor:"
            ]),
            ("2. Universal Node timeIn Propagation Architecture", [
                "Every node in Time Dilation DAW features an explicit or implicit Inlet 0 designated timeIn (NodePortType::Time).",
                "Graph executes propagateTimeDilationHierarchy() prior to DSP processing:",
                "\\[ \\gamma_{\\text{node}} = \\gamma_{\\text{patched}} \\text{ if connected, else } \\gamma_{\\text{upstream}} \\text{ if unpatched} \\]",
                "Sample-rate advance step d\\tau_n = \\gamma_{\\text{node}}[n] / f_s modulates both sub-sample audio resampling and control phase accumulators."
            ])
        ]
    )


