# Demo profile and log

`Profiles/demo.eag` and `Logs/demo_05.03.26_10-00-00.csv` are a self-contained
demo for the app: three motor controllers (`MTR1..MTR3`) and a cabinet
(`CABINET`) exchanging 24-byte frames (marker `AA 55`, node id at byte 2,
checksum of bytes 3..22 in byte 23).

To use it, copy the folders next to the executable, where the app keeps its
`Profiles` and `Logs` directories (the executable directory), then pick
`demo.eag` in the profile menu and read `demo_05.03.26_10-00-00.csv` via the port
menu -> "Read from file".

The scenario covers run/ready/fault transitions and start/stop commands. Only
the status (byte 3) and command (byte 4) masks carry the "view in log" flag, so
the text log under the graph shows exactly those events instead of every
measured value.
