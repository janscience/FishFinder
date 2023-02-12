# Fishfinder Electrodes

Different types of fishfinder electrodes are used. Some with two
electrodes and some with three electrodes. They differ in their
spatial characteristics, strength of the signal during saerching and
recording and in their ability to suppress common noise.

We must consider two situations:

1. Searching phase: the fishfinder is in one of the lobes of the
   fish's dipole field.
2. Recording phase: you are (hopefully) close and in parallel to the
   fish with one electrode near the head and the other one near the
   tail.


## Dipole fishfinder

![dipole fishfinder](images/dipole-fishfinder.png)


The dipole fishfinder can be used in two configurations:

1. Single-channel that records the voltage between the lead and the
   reference electrode. The reference electrode is connected to ground
   of the amplifier/recorder. Results in dipolar characteristics
   ([R&uuml;diger
   Krahe](https://www.biologie.hu-berlin.de/en/groupsites/vhphysiol)
   style).

2. Two-channels that record each of the two electrodes against a far
   away common ground ([Jan Benda](https://bendalab.github.io/) style).
   During the recording phase one would subtract the two channels.


## Stereo fishfinder

Hopkins/Harned/Sullivan style.

![stereo fishfinder](images/sullivan-stereo-fishfinder-by-sophie-picq.jpg)

Three electrodes. See [How to build wand electrodes and fish finder
schematics](https://mormyrids.myspecies.info/en/node/473) on
[Mormyridae - African weakly electric
fishes](https://mormyrids.myspecies.info) by [John
Sullivan](https://mormyrids.myspecies.info/en/user/175).

It depends on how you use the three electrodes:

- Use the middle one as ground and do a differential recording between
  the front and back electrodes. This results in similar, but probably
  better characteristics as the single-ended recording of the dipole
  fishfinder.

- The middle electrode is the reference against which the front and
  the back electrode are measured. Results in two channels each with a
  dipolar characteristics:

  - Just sum the two signals up. This is really bad during the search
    phase, because the signals on both channels differ in sign and
    almost cancel each other out. During recording phase, the sum of
    both channels result in the same signal as from the single-channel
    dipole fishfinder. In both contexts, common noise and hum is
    canceled out, because it is picked up with opposite signs by the
    two channels.
  - Subtract them. Same search properties as dipole fishfinder. Really
    bad for recording because signal amplitude gets really
    low. Therefore, this configuration is not an option.
  - Record them both as two channels. Similar as the two-channel
    dipole fishfinder, but can not differentiate between search phase
    and head-tail configuration.

If you use a stereo jack for this three-electrode fishfinder, then the
center electrode should be connected to the sleeve (ground, bottom
end), the tip electrode to the tip (left, hot) and the back electrode
to the ring (right, cold). If this is plugged into a mono amplifier
(guitar amplifier), then the ring is short-circuited with ground. You
are left with a fishfinder that measures the tip electrode versus the
central ground electrode. The back electrode is useless und the
effective length of the fishfinder is halfed, resulting in signals of
much less amplitude.

## Comparison

| Fishfinder type:                  | 1-channel dipole  | 2-channel dipole | Stereo differential    | Stereo sum (1-channel) | 2-channel stereo |
| --------------------------------- | ----------------- | ---------------- | ---------------------- | ---------------------- | ---------------- |
| Spatial characteristics           | Dipole            | **Monopole**     | Dipole                 | Dipole                 | Dipole           |
| Sensitivity during searching      | Good              | **Best**         | Good                   | Very weak              | Weak             |
| Noise supression during searching | Yes               | No               | **Better**             | Better?                | Yes?             |
| Indication of head-tail recording | No                | **Yes**          | No                     | No                     | No               |
| Sensitivity during recording      | Good              | Good             | Good                   | Good                   | Good             |
| Noise supression during recording | Yes               | No (software)    | Yes                    | Yes                    | Yes              |
