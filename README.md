# CS130 UCR GRAPHICS PIPELINE
The modified code for this project is located in the driver_state.cpp file.
## Description

This project simulates elements of the OpenGL graphics pipeline. This includes:
- triangle, indexed, triangle fan, and triangle strip vertex ordering schemes
- triangle clipping with the view-volume
- triangle color interpolation methods
- z-buffering
- triangle rasterization

## Usage

Compile the project with `scons` and then run each test ( 0 - 25 ) with the following:
`./driver -i XX.txt`
Where XX is in the range 00-25.

The outputted image will be stored in `output.png`
The reference images can be seen in 00.png - 25.png
The project can be compared to the grading script with `./grading-script.py .`.

## Example Outputs
![output1](https://github.com/DishonJordan/Graphics-Pipeline/blob/master/18.png)

![output2](https://github.com/DishonJordan/Graphics-Pipeline/blob/master/23.png)

![output2](https://github.com/DishonJordan/Graphics-Pipeline/blob/master/24.png)
# opPipe
