import os
import numpy as np
import matplotlib.pyplot as plt
from plotstyle import plot_style, spines_params, Pos
from thunderfish.efield import efish_monopoles, epotential_meshgrid, squareroot_transform
from thunderfish.efield import fieldline, plot_fieldlines
from thunderfish.efield import epotential, efield
from thunderfish.fishshapes import plot_fish, plot_fishfinder


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
    # field lines:
    filename = 'eodcircuit-fieldlines.npz'
    if not os.path.isfile(filename):
        # compute:
        print('compute field lines:')
        bounds = [[minx, miny], [maxx, maxy]]
        flines = {}
        for x0, y0 in ((0.5, 11), (0.5, 9.5), (0.5, 8.5),
                       (0.5, 7), (0.5, 4.5), (2, -11)):
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
    plot_fieldlines(ax, flines, 12, **s.lsFieldline)
    flines = []
    for d in data.files:
        dd = data[d]
        dd[:,0] *= -1
        flines.append(dd)
    plot_fieldlines(ax, flines, 5, **s.lsFieldline)
    # fish:
    plot_fish(ax, *fish, bodykwargs=s.fishLightBody,
              finkwargs=s.fishTransparentFins,
              eyekwargs=s.fishEyeLine)
    ax.set_xlim(minx, maxx)
    ax.set_ylim(miny, maxy)

    
def plot_eodcircuit(ax):
    rf1, rf2 = ax.resistance_v((0, 2.6), r'$R_f$')
    ef1, ef2 = ax.battery_v(rf1.down(4.8), r'$E_f$')
    #rw1, rw2 = ax.resistance_v(rf1.lefts(1.5).down(1.2), r'$R_w$')
    eod1 = ax.node((ef1.downs(1)), r'$V_t$', 'below')
    eod2 = ax.node(rf2.ups(0.8), r'$V_h$', 'above')
    ax.connect((eod1, ef1, None, ef2, rf1, None, rf2, eod2)) #, rw2, None, rw1))
    rl1, rl2 = ax.resistance_h(eod1.down(2.8).right(8.5), r'$R_w$')
    rl3, rl4 = ax.resistance_h(eod2.up(0.9).right(8), r'$R_w$')
    rv = Pos(19, -1.3)
    rl5, rl6 = ax.resistance(rv.down(4).left(1), 60, r'$R_w$', 'below')
    rl7, rl8 = ax.resistance(rv.up(4).left(1), 180-60, r'$R_w$', 'below')
    n1 = ax.node((14, -8.9))
    n2 = ax.node((14, 6.2))
    ax.connect_straight((eod1, rl1, None, rl2, n1, rl5, None, rl6,
                         rl7, None, rl8, n2, rl4, None, rl3, eod2))
    #return eod1, eod2
    return eod1, eod2, n1, n2, rv

    
def plot_fishfinder_circuit(ax, s, eod1, eod2, rv, central_ground=False):
    oppos = rv.right(12)
    opn, opp, opout, opgnd, oppwr = ax.opamp_l(oppos)
    ax.connect((eod2, opp, None, opout, opout.rights(0.5)))
    ax.connect((opn, eod1))
    ngnd = ax.node(oppos.downs(4))
    gnd = ax.ground(ngnd.downs(1))
    ax.connect((opgnd, gnd))
    b1, b2 = ax.battery_v(ngnd.rights(1).ups(1))
    ax.connect((ngnd, b1))
    plot_fishfinder(ax, rv.left(5), (0, 1), 15, central_ground=central_ground,
                    rodkwargs=dict(edgecolor='none', facecolor=s.palette['gray']),
                    poskwargs=dict(edgecolor='none', facecolor=s.palette['red']),
                    negkwargs=dict(edgecolor='none', facecolor=s.palette['blue']),
                    gndkwargs=dict(edgecolor='none', facecolor=s.palette['black']))
    ax.text(eod1.lefts(0.8).x(), opout.y(), 'electrodes',
            rotation='vertical', va='center')
    ax.add_patch(plt.Rectangle(opp.lefts(1.2).ups(0.7), 11, -19.5,
                           zorder=50, edgecolor=s.palette['black'],
                           facecolor=s.palette['gray']))
    ax.text(b2.x(), b2.ups(0.5).y(), 'recorder',
            rotation='vertical', zorder=60)
    return ngnd, opn, oppos

    
def plot_farground(ax, eod1, ngnd):
    rpos = (eod1.x() + 9, ngnd.y() + 0.4)
    rg1, rg2 = ax.resistance(rpos, -18, r'$R_{g}$', 'below', va='top')
    ax.connect_straight((eod1, rg1, None, rg2, ngnd))
    tip = ngnd.left(17)
    ax.plot([tip.x(), ngnd.x()], [tip.y(), ngnd.y()],
            s.palette['gray'], lw=4, solid_capstyle='butt', zorder=20)
    ax.node(tip)
    ax.text(ngnd.left(16.5).x(), ngnd.y() + 1, 'ground wire')
    
    
def plot_singleended(ax, opn, ngnd):
    n1 = ax.node(opn.lefts(0.7))
    ax.connect((n1, ngnd))

    
def plot_pseudodifferential(ax, opn, ngnd):
    n1 = ax.node(opn.lefts(0.7))
    rp1, rp2 = ax.resistance_v(n1.downs(1.8), r'$R_p$')
    ax.connect((n1, rp2, None, rp1, ngnd))

    
def plot_centralground(ax, oppos, ngnd):
    n1 = oppos.lefts(5.5)
    n2 = ngnd.lefts(1)
    ax.connect((ngnd, n2, n1))

    
if __name__ == "__main__":
    s = plot_style()

    fig, ax = plt.subplots(cmsize=(8.0, 5.5))
    fig.subplots_adjust(nomargins=True)
    plot_efish(ax, s)
    eod1, eod2, e1, e2, rv = plot_eodcircuit(ax)
    t = ax.text(0.97, 0.99, 'Dipole field of electric fish',
                transform=ax.transAxes, ha='right', va='top')
    fig.savefig()

    ngnd, opn, oppos = plot_fishfinder_circuit(ax, s, e1, e2, rv)
    t.set_text('Floating differential')
    fig.savefig('eodcircuit-fishfinder')

    # far ground:
    plot_farground(ax, eod1, ngnd)
    t.set_text('Differential with ground electrode')
    fig.savefig('eodcircuit-farground')

    # single ended:
    fig, ax = plt.subplots(cmsize=(8.0, 5.5))
    fig.subplots_adjust(nomargins=True)
    plot_efish(ax, s)
    eod1, eod2, e1, e2, rv = plot_eodcircuit(ax)
    ngnd, opn, oppos = plot_fishfinder_circuit(ax, s, e1, e2, rv)
    plot_singleended(ax, opn, ngnd)
    ax.text(0.97, 0.99, 'Single ended',
            transform=ax.transAxes, ha='right', va='top')
    fig.savefig('eodcircuit-se')

    # pseudo-differential:
    fig, ax = plt.subplots(cmsize=(8.0, 5.5))
    fig.subplots_adjust(nomargins=True)
    plot_efish(ax, s)
    eod1, eod2, e1, e2, rv = plot_eodcircuit(ax)
    ngnd, opn, oppos = plot_fishfinder_circuit(ax, s, e1, e2, rv)
    plot_pseudodifferential(ax, opn, ngnd)
    ax.text(0.97, 0.99, 'Pseudo differential',
            transform=ax.transAxes, ha='right', va='top')
    fig.savefig('eodcircuit-pseudodiff')

    # central ground (Hopkins - style):
    fig, ax = plt.subplots(cmsize=(8.0, 5.5))
    fig.subplots_adjust(nomargins=True)
    plot_efish(ax, s)
    eod1, eod2, e1, e2, rv = plot_eodcircuit(ax)
    ngnd, opn, oppos = plot_fishfinder_circuit(ax, s, e1, e2, rv, True)
    plot_centralground(ax, oppos, ngnd)
    ax.text(0.97, 0.99, 'Central ground electrode',
            transform=ax.transAxes, ha='right', va='top')
    fig.savefig('eodcircuit-centralground')
