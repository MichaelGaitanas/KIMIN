import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path

MAP_FILE  = "stability_map.txt"
INFO_FILE = "batch_info.txt"   # contains: Nmax_global  ord  (tmax - t0)


def main():
    # ---------------------------------------------------------------
    # 1) Load stability map: columns = a0, e0, value
    # ---------------------------------------------------------------
    data = np.loadtxt(MAP_FILE)
    a0   = data[:, 0]
    e0   = data[:, 1]
    val  = data[:, 2]

    a_vals = np.unique(a0)
    e_vals = np.unique(e0)
    Na = a_vals.size
    Ne = e_vals.size

    if Na * Ne != val.size:
        raise RuntimeError("Grid is not rectangular: Na * Ne != number of rows.")

    # Shape: rows -> a index, columns -> e index
    V = val.reshape(Na, Ne)

    # ---------------------------------------------------------------
    # 2) Identify collision types and mask them out of the heatmap
    # ---------------------------------------------------------------
    coll_bin = np.isclose(V, -1.0)
    coll_b1  = np.isclose(V, -2.0)
    coll_b2  = np.isclose(V, -3.0)

    V_plot = V.astype(float)
    V_plot[coll_bin | coll_b1 | coll_b2] = np.nan  # keep heatmap for non-collisions

    # ---------------------------------------------------------------
    # 3) Read batch_info for title (Nmax, order, total time)
    # ---------------------------------------------------------------
    title = ""
    info_path = Path(INFO_FILE)
    if info_path.exists():
        info = np.loadtxt(INFO_FILE)
        # Expect: Nmax_global, ord, tmax_minus_t0
        Nmax = int(info[0])
        ordV = int(info[1])
        Tsec = float(info[2])
        Tdays = Tsec / 86400.0
        title = rf"$N_{{\max}} = {Nmax}$,  $T = {Tdays:.2f}\,\mathrm{{days}}$,  order of $V = {ordV}$"

    # ---------------------------------------------------------------
    # 4) Plot: eccentricity on x-axis, a on y-axis
    # ---------------------------------------------------------------
    fig, ax = plt.subplots(figsize=(6, 5))

    extent = (e_vals.min(), e_vals.max(), a_vals.min(), a_vals.max())

    cmap = plt.get_cmap("viridis").copy()
    cmap.set_bad("black")  # NaNs (collisions) as black background

    im = ax.imshow(
        V_plot,
        origin="lower",
        extent=extent,
        aspect="auto",
        cmap=cmap
    )

    cbar = fig.colorbar(im, ax=ax)
    cbar.set_label("max |Δr| on x–z section [km]")

    ax.set_xlabel(r"$e_0$")
    ax.set_ylabel(r"$a_0$  [km]")

    if title:
        ax.set_title(title)

    # ---------------------------------------------------------------
    # 5) Overplot collisions as coloured markers with legend
    # ---------------------------------------------------------------
    if np.any(coll_bin | coll_b1 | coll_b2):
        aa, ee = np.meshgrid(a_vals, e_vals, indexing="ij")

        if np.any(coll_bin):
            ax.scatter(
                ee[coll_bin], aa[coll_bin],
                s=12, c="cyan", marker="x",
                label="collision (binary)"
            )
        if np.any(coll_b1):
            ax.scatter(
                ee[coll_b1], aa[coll_b1],
                s=12, c="red", marker="^",
                label="collision with B1"
            )
        if np.any(coll_b2):
            ax.scatter(
                ee[coll_b2], aa[coll_b2],
                s=12, c="orange", marker="s",
                label="collision with B2"
            )

        ax.legend(loc="upper right", fontsize=8)

    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    main()