# RHMQ: RightHook Message Queue

This lib is a wrapper of zmq and Google flatbuffers for convenient communication of RightHook formatted data.

The required includes are: `<REPO>/include`, `<REPO>/thirdparty/flatbuffers/include`, and `<REPO>/thirdparty/zmq/include`

The required linkages are: `<REPO>/thirdparty/zmq/lib/<PLATFORM>`

See the appropriate readmes for the respective thirdparties in their folders under `<REPO>/thirdparty`

## Schemas

(instructions also included in the flatbuffer README.md)

RightHook schemas are stored in folder: `<REPO>/schema`

Run `make` inside that folder, under linux, to update the generated headers that are placed in `<REPO>/include/generated`
