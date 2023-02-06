import os
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.transforms as mpt
from matplotlib.patches import Rectangle
from plotstyle import plot_style, spines_params
from thunderfish.efield import efish_monopoles, epotential_meshgrid, squareroot_transform
from thunderfish.efield import fieldline, plot_fieldlines
from thunderfish.efield import epotential, efield
from thunderfish.fishshapes import plot_fish


def plot_efish(ax, s):
    scale = 14.0
    miny = -1.3*scale
    maxy = scale
    minx = -0.5*scale
    maxx = minx + (maxy-miny)/ax.aspect_ratio()
    x = np.linspace(minx, maxx, 13*int(maxx))
    y = np.linspace(miny, maxy, 10*int(maxy))
    xx, yy = np.meshgrid(x, y)
    fish = (('Eigenmannia', 'side'), (0, 2), (0, 1), 18.0, 0)
    poles = efish_monopoles(*fish[1:])
    pot = epotential_meshgrid(xx, yy, None, poles)
    mz = 0.9
    zz = squareroot_transform(pot/200, mz)
    levels = np.linspace(-mz, mz, 25)
    ax.show_spines('')
    ax.contourf(x, y, -zz, levels, zorder=0)
    ax.contour(x, y, -zz, levels, zorder=1, **s.csLine)
    """
    # field lines:
    filename = 'eodcircuit-fieldlines.npz'
    if not os.path.isfile(filename):
        # compute:
        print('compute field lines:')
        bounds = [[minx, miny], [maxx, maxy]]
        flines = {}
        for x0, y0 in ((minx+1, 0.2*maxy), (0.7*minx, maxy), (-5, 12), (-5, 6),
                       (0.3*maxx, maxy), (0.8*maxx, maxy), (maxx-0.5, 0.3*maxy)):
            fl = fieldline((x0, y0), bounds, poles, eps=0.2, maxiter=5000)
            flines['left%02d' % len(flines)] = fl
            print('%4.0f, %4.0f: %d' % (x0, y0, len(fl)))
        np.savez(filename, **flines)
    # plot field lines:
    data = np.load(filename)
    flines = []
    for d in data.files:
        dd = data[d]
        flines.append(dd)
    plot_fieldlines(ax, flines, 0.3*maxx, **s.lsFieldline)
    flines = []
    for d in data.files:
        dd = data[d]
        dd[:,1] *= -1
        flines.append(dd)
    plot_fieldlines(ax, flines, 0.3*maxx, **s.lsFieldline)
    """
    # fish:
    plot_fish(ax, *fish, bodykwargs=s.fishLightBody,
              finkwargs=s.fishTransparentFins,
              eyekwargs=s.fishEyeLine)
    ax.set_xlim(minx, maxx)
    ax.set_ylim(miny, maxy)

    
def plot_fishfinder(ax, s, pos, direction, length, central_ground=False):
    width = 6
    transform = mpt.Affine2D().rotate(np.arctan2(direction[1], direction[0])).translate(*pos) + ax.transData
    ax.plot([0, length], [0, 0], color=s.palette['gray'], lw=width,
            transform=transform, solid_capstyle='butt', zorder=50)
    ax.plot([1, 2], [0, 0], color=s.palette['blue'], lw=1.3*width,
            transform=transform, solid_capstyle='butt', zorder=52)
    ax.plot([length-1, length-2], [0, 0], color=s.palette['red'], lw=1.3*width,
            solid_capstyle='butt', transform=transform, zorder=52)
    #ax.plot([length-2, 0], [0, 0], color=s.palette['red'], lw=s.lwthin,
    #        solid_capstyle='butt', transform=transform, zorder=51)
    #ax.plot([2, 0], [-0.2, -0.2], color=s.palette['blue'], lw=s.lwthin,
    #        solid_capstyle='butt', transform=transform, zorder=51)
    if central_ground:
        ax.plot([length/2-0.5, length/2+0.5], [0, 0], color=s.palette['black'],
                lw=1.3*width, solid_capstyle='butt', transform=transform,
                zorder=52)

    
def plot_eodcircuit(ax):
    rf1, rf2 = ax.resistance_v((0, 3.5), r'$R_f$')
    ef1, ef2 = ax.battery_v(rf1.downs(1), r'$E_f$')
    rw1, rw2 = ax.resistance_v(rf1.lefts(1.5), r'$R_w$')
    eod1 = ax.node((ef1.downs(1)))
    eod2 = ax.node(rf2.ups(1))
    ax.connect((ef2, rf1, None, rf2, eod2, rw2, None, rw1, eod1, ef1))
    return eod1, eod2

    
def plot_fishfinder_circuit(ax, s, eod1, eod2, central_ground=False):
    rm1, rm2 = ax.resistance_h(eod1.rights(5.5), r'$R_{m1}$')
    rm3, rm4 = ax.resistance_h(eod2.rights(5.5), r'$R_{m2}$')
    n1 = ax.node(rm2.rights(2))
    n2 = ax.node(rm4.rights(2))
    oppos = n1.rights(3).ups(2.07)
    opn, opp, opout, opgnd = ax.opamp_l(oppos)
    ax.connect((eod2, rm3, None, rm4, n2, opp, None, opout, opout.rights(0.5)))
    ax.connect((opn, n1, rm2, None, rm1, eod1))
    ngnd = ax.node(oppos.downs(5))
    gnd = ax.ground(ngnd.downs(1))
    ax.connect((opgnd, gnd))
    b1, b2 = ax.battery_v(ngnd.rights(1).ups(1))
    ax.connect((ngnd, b1))
    plot_fishfinder(ax, s, n1.downs(0.5), (0, 1), 15.4, central_ground)
    ax.text(n1.lefts(0.8).x(), opout.y(), 'electrodes',
            rotation='vertical', va='center')
    ax.add_patch(Rectangle(opp.lefts(1).ups(0.7), 10.5, -22.3,
                           zorder=50, edgecolor=s.palette['black'],
                           facecolor=s.palette['gray']))
    ax.text(b2.x(), b2.ups(0.5).y(), 'recorder',
            rotation='vertical', zorder=60)
    return ngnd, opn, oppos

    
def plot_farground(ax, eod1, eod2, ngnd):
    n1 = ax.node(eod1.rights(2))
    n2 = ax.node(eod2.rights(4))
    rg1, rg2 = ax.resistance_v(n1.downs(1.5), r'$R_{g1}$')
    rg3, rg4 = ax.resistance_v(n1.downs(1.5).rights(2), r'$R_{g2}$')
    n3 = ax.node(rg3.downs(1))
    ax.connect((n1, rg2, None, rg1, ngnd))
    ax.connect((n2, rg4, None, rg3, n3))
    ax.plot([n3.rights(0.5).x(), ngnd.x()], [n3.y(), n3.y()],
            s.palette['gray'], lw=4, zorder=20)
    ax.text(n3.rights(1).x(), n3.y() + 1, 'ground wire')
    
    
def plot_singleended(ax, opn, ngnd):
    n1 = ax.node(opn.lefts(0.5))
    ax.connect((n1, ngnd))

    
def plot_centralground(ax, oppos, ngnd):
    n1 = oppos.lefts(3)
    n2 = ngnd.lefts(1)
    ax.connect((ngnd, n2, n1))

    
if __name__ == "__main__":
    s = plot_style()

    fig, ax = plt.subplots(cmsize=(8.0, 5.5))
    fig.subplots_adjust(nomargins=True)
    plot_efish(ax, s)
    eod1, eod2 = plot_eodcircuit(ax)
    t = ax.text(0.03, 0.03, 'Dipole field of electric fish',
                transform=ax.transAxes)
    fig.savefig()
    ngnd, opn, oppos = plot_fishfinder_circuit(ax, s, eod1, eod2)
    t.set_text('Floating differential')
    fig.savefig('eodcircuit-fishfinder')

    # far ground:
    plot_farground(ax, eod1, eod2, ngnd)
    t.set_text('Differential with ground electrode')
    fig.savefig('eodcircuit-farground')

    # single ended:
    fig, ax = plt.subplots(cmsize=(8.0, 5.5))
    fig.subplots_adjust(nomargins=True)
    plot_efish(ax, s)
    eod1, eod2 = plot_eodcircuit(ax)
    ngnd, opn, oppos = plot_fishfinder_circuit(ax, s, eod1, eod2)
    plot_singleended(ax, opn, ngnd)
    ax.text(0.03, 0.03, 'Single ended',
            transform=ax.transAxes)
    fig.savefig('eodcircuit-se')

    # central ground (Hopkins - style):
    fig, ax = plt.subplots(cmsize=(8.0, 5.5))
    fig.subplots_adjust(nomargins=True)
    plot_efish(ax, s)
    eod1, eod2 = plot_eodcircuit(ax)
    ngnd, opn, oppos = plot_fishfinder_circuit(ax, s, eod1, eod2, True)
    plot_centralground(ax, oppos, ngnd)
    ax.text(0.03, 0.03, 'Central ground electrode',
            transform=ax.transAxes)
    fig.savefig('eodcircuit-centralground')
    
