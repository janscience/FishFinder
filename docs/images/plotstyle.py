import matplotlib as mpl
import plottools.plottools as pt
from plottools.colors import lighter
from plottools.spines import spines_params

    
def plot_style():
    palette = pt.palettes['muted']
    palette['black'] = pt.palettes['unituebingen']['black']
    class ns: pass
    ns.lwthick = 1.8
    ns.lwthin = 1.0
    ns.lwspines = 1.0
    pt.generic_styles(colors='muted', lwthick=ns.lwthick, lwthin=ns.lwthin,
                      markerlarge=6.5, markersmall=4.0, mec=0.0, mew=0.8,
                      fillalpha=0.4, namespace=ns)
    pt.make_line_styles(ns, 'ls', 'Spine', '', palette['black'], '-', ns.lwspines,
                        clip_on=False)
    pt.make_line_styles(ns, 'ls', 'Grid', '', palette['gray'], '--', 0.7*ns.lwthin)
    pt.make_line_styles(ns, 'ls', 'Dotted', '', palette['gray'], ':', 0.7*ns.lwthin)
    pt.make_line_styles(ns, 'ls', 'Marker', '', palette['black'], '-', ns.lwthick,
                     clip_on=False)
    pt.make_line_styles(ns, 'ls', 'Lat', '', palette['lightgreen'], '-', ns.lwthick)
    pt.make_line_styles(ns, 'ls', 'Diag', '', palette['green'], '-', ns.lwthick)
    pt.make_line_styles(ns, 'ls', 'Front', '', palette['cyan'], '-', ns.lwthick)
    pt.make_line_styles(ns, 'ls', 'Lat', 'd', palette['lightgreen'], '--', ns.lwthin)
    pt.make_line_styles(ns, 'ls', 'Diag', 'd', palette['green'], '--', ns.lwthin)
    pt.make_line_styles(ns, 'ls', 'Front', 'd', palette['cyan'], '--', ns.lwthin)
    pt.make_line_styles(ns, 'ls', 'ExpLat', '', palette['blue'], '-', ns.lwthick)
    pt.make_line_styles(ns, 'ls', 'ExpDiag', '', palette['purple'], '-', ns.lwthick)
    pt.make_line_styles(ns, 'ls', 'ExpFront', '', palette['orange'], '-', ns.lwthick)
    pt.make_line_styles(ns, 'ls', 'Visual', '', palette['green'], '-', ns.lwthick)
    pt.make_line_styles(ns, 'ls', 'Comm', '', palette['blue'], '-', ns.lwthick)
    pt.make_line_styles(ns, 'ls', 'Navi', '', palette['purple'], '-', ns.lwthick)
    pt.make_line_styles(ns, 'ls', 'Local', '', palette['red'], '-', ns.lwthick)
    pt.make_line_styles(ns, 'ls', 'EOD', '', palette['blue'], '-', 0.7)
    pt.make_line_styles(ns, 'ls', 'EOD2', '', palette['purple'], '-', 0.7)
    pt.make_line_styles(ns, 'ls', 'Fieldline', '', palette['purple'], '-', 0.7)
    pt.arrow_style(ns, 'Line', dist=3.0, style='>', shrink=0, lw=0.6,
                color=palette['black'], head_length=4, head_width=4)
    pt.arrow_style(ns, 'Hertz', dist=3.0, style='>', shrink=0, lw=0.6,
                color=palette['black'], head_length=3, head_width=3,
                heads='<>', text='%.0f\u2009Hz', rotation='vertical', fontsize='x-small',
                bbox=dict(boxstyle='round,pad=0.1', facecolor='white', edgecolor='none',
                          alpha=0.6))
    pt.arrow_style(ns, 'Point', dist=3.0, style='>>', shrink=0, lw=1.2,
                color=palette['black'], head_length=8, head_width=6)
    pt.arrow_style(ns, 'PointSmall', dist=3.0, style='>>', shrink=0, lw=1,
                color=palette['black'], head_length=5, head_width=5,
                fontsize='small')
    pt.arrow_style(ns, 'Marker', dist=3.0, style='>>', shrink=0, lw=0.9,
                color=palette['black'], head_length=5, head_width=5,
                fontsize='small', ha='center', va='center',
                bbox=dict(boxstyle='round, pad=0.1, rounding_size=0.4',
                          facecolor=palette['white'], edgecolor='none', alpha=0.6))
    ns.tsEOD = dict(fontsize='x-small',
                    bbox=dict(boxstyle='round,pad=0.1', facecolor='white',
                              edgecolor='none', alpha=0.6))
    ns.csLine = dict(colors=palette['gray'], linewidths=0.1, linestyles='solid')
    ns.fishBody = dict(zorder=21, lw=0.1, edgecolor=palette['black'],
                       facecolor=palette['black'])
    ns.fishFins = dict(zorder=20, lw=0.5, alpha=0.4,
                       edgecolor=pt.lighter(palette['black'], 0.4),
                       facecolor=palette['black'])
    ns.fishOpenBody = dict(zorder=21, lw=0.2,
                           edgecolor=palette['black'], facecolor='none')
    ns.fishLightBody = dict(zorder=21, lw=0.2, edgecolor=palette['black'],
                            facecolor=pt.lighter(palette['black'], 0.2))
    ns.fishLightFins = dict(zorder=20, lw=0.2, edgecolor=palette['black'],
                            facecolor=pt.lighter(palette['black'], 0.05))
    ns.fishTransparentFins = dict(zorder=20, lw=0.2, alpha=0.5,
                                  edgecolor=palette['black'],
                                  facecolor=lighter(palette['black'], 0.1))
    ns.fishEyeLine = dict(zorder=22, lw=0.2, edgecolor=palette['black'],
                          facecolor='none')
    
    # rc settings:
    cmcolors = [palette['red'], pt.lighter(palette['orange'], 0.85),
                pt.lighter(palette['yellow'], 0.2), pt.lighter(palette['lightblue'], 0.8),
                palette['blue']]
    cmvalues = [0.0, 0.25, 0.5, 0.8, 1.0]
    pt.colormap('RYB', cmcolors, cmvalues)
    #mpl.rcParams['image.cmap'] = 'RYB'
    #mpl.rcParams['image.cmap'] = 'RdYlBu'
    cmcolors = [pt.lighter(palette['yellow'], 0.2), pt.lighter(palette['orange'], 0.85),
                palette['red']]
    cmvalues = [0.0, 0.5, 1.0]
    pt.colormap('YR', cmcolors, cmvalues)
    cmcolors = [pt.lighter(palette['yellow'], 0.2), palette['green'],
                palette['darkgreen'], pt.darker(palette['darkgreen'], 0.5)]
    cmvalues = [0.0, 0.6, 0.8, 1.0]
    pt.colormap('YG', cmcolors, cmvalues)
    pt.axes_params(xmargin=0, ymargin=0, zmargin=0, color='none')
    cycle_colors = ['blue', 'red', 'orange', 'lightgreen', 'magenta', 'yellow', 'cyan', 'pink']
    pt.circuits_params(scale=3, connectwidth=0.8, linewidth=1.2,
                       color=palette['black'],
                       facecolor=palette['white'], alpha=0.5,
                       zorder=100)
    pt.colors_params(ns.palette, cycle_colors, cmap='RdYlBu')
    pt.figure_params(color='none', format='pdf',
                  compression=6, fonttype=3, stripfonts=False)
    pt.grid_params(grid=False, axis='both', which='major', **ns.lsGrid)
    pt.labels_params(labelformat='{label} [{unit}]', labelsize='small', labelweight='normal',
                  labelcolor='axes', labelpad=4,
                  xlabellocation='center', ylabellocation='center')
    pt.legend_params(fontsize='small', frameon=False, borderpad=0,
                  handlelength=1.5, handletextpad=0.5,
                  numpoints=1, scatterpoints=1, labelspacing=0.5, columnspacing=0.5)
    pt.scalebars_params(format_large='%.0f', format_small='%.1f', lw=2.0, color=palette['black'],
                    capsize=0, clw=0.5, font=dict(fontsize='small', fontstyle='normal'))
    pt.spines_params(spines='lb', spines_offsets={'lrtb': 3}, spines_bounds={'lrtb': 'full'},
                  color=ns.lsSpine['color'], linewidth=ns.lsSpine['linewidth'])
    pt.tag_params(xoffs='auto', yoffs='auto', label='%A', minor_label='%A$_{\text{%mi}}$',
               font=dict(fontsize='x-large', fontstyle='normal', fontweight='normal'))
    pt.text_params(font_size=8, font_family='sans-serif', color='axes')
    pt.ticks_params(xtick_minor=False, xtick_dir='out', xtick_size=3.5, minor_tick_frac=0.6,
                 xtick_major_width=None, xtick_minor_width=None, xtick_major_pad=None,
                 xtick_alignment='center', ytick_alignment='center_baseline',
                 xtick_color='axes', xtick_labelcolor='ticks', xtick_labelsize='medium')
    return ns

