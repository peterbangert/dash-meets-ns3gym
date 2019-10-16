# RL Approach to HAS algorithms

> Based off the work from Harald Ott : Simulation Framework for HTTP-Based Adaptive Streaming Applications

### Reference Repo: [haraldott/dash](https://github.com/haraldott/dash)


### Running Dash code


`
./waf --run="tcp-stream --simulationId=1 --numberOfClients=1 --adaptationAlgo=rl-algorithm --segmentDuration=2000000 --segmentSizeFile=contrib/dash/segmentSizes.txt"
`

`
export NS_LOG="RLAlgorithm"
`

### Dash arguments

- simulationId: The Id of this simulation, to distinguish it from others, with same algorithm and number of clients, for logging purposes.
- numberOfClients: The number of streaming clients used for this simulation.
- segmentDuration: The duration of a segment in microseconds.
- adaptationAlgo: The name of the adaptation algorithm the client uses for the simulation. The 'pre-installed' algorithms are tobasco, festive and panda.
- segmentSizeFile: The relative path (from the ns-3.x/ folder) of the file containing the sizes of the segments of the video. The segment sizes have to be provided as a (n, m) matrix, with n being the number of representation levels and m being the total number of segments. A two-segment long, three representations containing segment size file would look like the following: