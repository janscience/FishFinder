# FishFinder

Smart fishfinders for better EOD recordings of electric fish in the field.

Single channel, dipole fishfinder:

- [plain fishfinder](fishfinder-plain/)
- [fishfinder with display](fishfinder-display/)

Two channel, monopole fishfinder:

- [fishfinder](fishfinder/)

## Power consumption

Without amplifier:

|           | Voltage | Current | Power | Runtime 2Ah |
| :-------- | ------: | ------: | ----: | ----------: |
| running   |    3.7V |   114mA | 420mW |         15h |
| recording |    3.7V |   136mA | 505mW |       12.5h |
| running   |      5V |   145mA | 720mW |        8.5h |
| recording |      5V |   153mA | 770mW |          8h |

The last colum is the run time to be expected for a 2Ah battery
assuming an efficiancy of 85%: for the 3.7V LiPo it is 2Ah times times
85% efficiancy divided by current, for the 5V power bank it is 2Ah
times 3.7V LiPo voltage divided by 5.1V output voltage times 85%
efficiancy (=0.62) divided by current.

## Documentation

- [Fishfinder electrodes](docs/electrodes.md)



