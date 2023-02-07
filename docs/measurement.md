# How to measure EODs

## Dipole field

![eodfield](images/eodcircuit.png)

Electric fish generate an electric dipole field around their body
(isopotential lines in color). The electric organ can be approximated
by a battery *E<sub>f</sub>* in series with a resistance
*R<sub>f</sub>*. The current then flows along all the field lines
through the water, which acts like a resistance *R<sub>w</sub>*.

The water resistance is inversly related to the water
conductivity. Typical values range from *R<sub>w</sub>* = 5k&#8486; in
300&micro;S/cm water to *R<sub>w</sub>* = 300k&#8486; in 5&micro;S/cm.

The fish inner resistance *R<sub>f</sub>* is similar or higher than
the typical water conductivity in the habitat of the fish (precise
data are not available). Thus the voltage that can be measured between
the head and the tail of the fish (filled circles) is less than half
of the fish battery *E<sub>f</sub>*. Values of the fish battery start
at around 50mV in *Apteronotus* and go all the way to 800V for
*Electrophorus*. Because of the voltage divider circuit we we measure
just a few Millivolts for *Apteronotus* and 10V for *Electrophorus* in
the water.

During each discharge of the electric organ the fish battery follows a
species specific time course that may include reversal of
polarity. The values for *E<sub>f</sub>* mentioned above are just the
maximum amplitudes of these waveforms.

It is the shape of this waveform that we want to measure.

Wavetype electric fish discharge their electric organ in regular
succession such that the time course of *E<sub>f</sub>* is a periodic
waveform with a fundamental frequency between less than 100Hz over
1000Hz up to more than 2kHz.

| Quantity        | Typical values          |
| --------------- | ----------------------- |
| *R<sub>w</sub>* | 5k&#8486; - 300k&#8486; |
| *R<sub>f</sub>* | > *R<sub>w</sub>*       |
| *E<sub>f</sub>* | 50mV, 200mV, 800V       |

## Fishfinder

![fishfinder](images/eodcircuit-fishfinder.png)

For measuring the EOD we use a fishfinder electrode - a rod with two
(or three - see below) electrodes connected to an amplifier. The
electrodes will be at some distance to the fish.


## Single ended measurement

![single ended](images/eodcircuit-se.png)


## Differential recording far away ground wire

![far ground](images/eodcircuit-farground.png)


## Differential recording with third electrode on fishfinder

![central ground](images/eodcircuit-centralground.png)
