import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path
from matplotlib.colors import LogNorm
from matplotlib.colors import ListedColormap, Normalize
from matplotlib.patches import Patch

MAP_POS_FILE  = "io/stability_map_pos.txt"   # run with e>=0, M_sp, w_sp for e>0 branch
MAP_NEG_FILE  = "io/stability_map_neg.txt"   # run with e>=0, M_sp, w_sp for e<0 branch
INFO_POS_FILE = "io/batch_info_pos.txt"      # from the "pos" run


def load_map(fname):
    data = np.loadtxt(fname)
    a0   = data[:, 0]
    e0   = data[:, 1]
    val  = data[:, 2]

    a_vals = np.unique(a0)
    e_vals = np.unique(e0)
    Na = a_vals.size
    Ne = e_vals.size

    if Na * Ne != val.size:
        raise RuntimeError(f"{fname}: grid is not rectangular")

    V = val.reshape(Na, Ne)
    return a_vals, e_vals, V


def main():
    # -----------------------------------------------------------
    # 1) Load both halves
    # -----------------------------------------------------------
    a_pos, e_pos, V_pos = load_map(MAP_POS_FILE)
    a_neg, e_neg, V_neg = load_map(MAP_NEG_FILE)

    # Sanity checks
    if not np.allclose(a_pos, a_neg):
        raise RuntimeError("a-grids differ between pos/neg maps")
    if not np.allclose(e_pos, e_neg):
        raise RuntimeError("e-grids differ between pos/neg maps")

    a_vals = a_pos
    e_vals = e_pos  # non-negative grid, assumed ascending and including 0
    Na = a_vals.size
    Ne = e_vals.size

    # -----------------------------------------------------------
    # 2) Build signed-e grid
    # -----------------------------------------------------------
    # Use e>=0 grid on the right; mirror it to the left (excluding 0 twice)
    e_neg_signed = -e_vals[1:][::-1]        # (-e_max ... -e_min), no zero
    e_signed = np.concatenate((e_neg_signed, e_vals))
    Ns = e_signed.size                      # = 2*Ne - 1

    # Construct full value matrix V_full (Na x Ns)
    V_full = np.full((Na, Ns), np.nan, dtype=float)

    # Left side (negative e): fill columns 0 .. Ne-2 with mirrored V_neg
    # V_neg is (Na x Ne) on e>=0; we want columns 1: reversed to map to e<0
    V_full[:, 0:Ne-1] = V_neg[:, 1:][:, ::-1]

    # Right side (e>=0): columns Ne-1 .. Ns-1
    V_full[:, Ne-1:] = V_pos

    # -----------------------------------------------------------
    # 3) Identify collisions and mask them out of the heatmap
    # -----------------------------------------------------------
    coll_bin = np.isclose(V_full, -1.0)
    coll_b1  = np.isclose(V_full, -2.0)
    coll_b2  = np.isclose(V_full, -3.0)

    V_plot = V_full.copy()
    V_plot[coll_bin | coll_b1 | coll_b2] = np.nan

    # -----------------------------------------------------------
    # 4) Read info from positive run
    # -----------------------------------------------------------
    title = ""
    info_path = Path(INFO_POS_FILE)
    if info_path.exists():
        info = np.loadtxt(INFO_POS_FILE)
        Nmax = int(info[0])
        ordV = int(info[1])
        Tsec = float(info[2])
        Tdays = Tsec / 86400.0
        title = rf"$\Delta t = {Tdays:.0f}\,\mathrm{{[days]}}$ per simulation (unless collision),     Order of $V = {ordV}$,     $i(0) = 91$ [deg],     $\Omega(0) = 270$ [deg]"

    # -----------------------------------------------------------
    # 5) Plot: eccentricity on x-axis, a on y-axis
    # -----------------------------------------------------------
    fig, ax = plt.subplots(figsize=(12, 9))   # 16:9 aspect ratio

    extent = (e_signed.min(), e_signed.max(), a_vals.min(), a_vals.max())

    cmap = plt.get_cmap("viridis").copy()
    cmap.set_bad("black")  # NaNs (collisions) → black

    im = ax.imshow(
        V_plot,
        origin="lower",
        extent=extent,
        aspect="auto",
        cmap=cmap,
        norm=LogNorm(vmin=np.nanmin(V_plot[V_plot > 0]),   # smallest positive
                    vmax=np.nanmax(V_plot))              # max value
    )

    cbar = fig.colorbar(im, ax=ax)
    cbar.set_label(r"Max |$\Delta_{xz}$| in [km] between the i.c. and all states", fontsize=16)

    ax.set_xlabel(r"$e(0) \rightarrow $ (negative rotates $\omega(0),M(0)$)", fontsize=16)
    ax.set_ylabel(r"$\alpha(0)$  [km]", fontsize=16)

    if title:
        ax.set_title(title)

    norm01 = Normalize(vmin=0.0, vmax=1.0)

    def overlay_mask(mask, color, label):
        arr = np.where(mask, 1.0, np.nan)   # 1 where collision, NaN elsewhere
        cmap = ListedColormap([color])
        cmap.set_bad((0, 0, 0, 0))          # NaNs transparent
        ax.imshow(
            arr,
            origin="lower",
            extent=extent,
            aspect="auto",
            cmap=cmap,
            norm=norm01,
        )
        return Patch(facecolor=color, edgecolor="none", label=label)

    handles = []
    if np.any(coll_bin):
        handles.append(overlay_mask(coll_bin, "cyan",    "Collision (binary)"))
    if np.any(coll_b1):
        handles.append(overlay_mask(coll_b1, "red", "Collision with B1"))
    if np.any(coll_b2):
        handles.append(overlay_mask(coll_b2, "orange",   "Collision with B2"))

    if handles:
        ax.legend(handles=handles, loc="upper right", fontsize=8)

    plt.tight_layout()
    plt.savefig('map_ord'+str(ordV)+'.png', dpi=200)

if __name__ == "__main__":
    main()