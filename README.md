# CIS-3110 Assignment 2
## CPU-Simulation - Christian Catalano, 1120832
### Compilation Instructions
- Navigate to the root directory  
- Run `make`

### Running the simulation
- Navigate to the root directory  
- Create an input file that matches the format provided in the assignment PDF  
    - Input file is validated on program launch
    - A test file `testfile.txt` is provided in the root directory
- run `./simcpu [-d] [-v] [-r] [quantum] < [inputfile]`, where
    - `[-d]` toggles detailed mode
    - `[-v]` toggles verbose mode
    - `[-r] [quantum]` toggles round robin mode with a time quantum of `[quantum]` time units
    - `[inputfile]` is the input file created in the previous step
    - input parameters are validated on program launch