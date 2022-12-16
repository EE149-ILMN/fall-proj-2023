# fall-proj-2023

### Setup
1. Install Docker
2. Clone tensorflow and tensor-lite micro repositories into ./src folder
3. Download data and extract motion and sleep data to ./data folder

### Opening Juypter Notebook
Run once to build docker container
```
docker build -t project:latest .
```
Run to startup docker container
```
docker run -itp 8888:8888 -v "/$(pwd)":/home/jovyan/work project:latest
```
Then open link generated in terminal

### Preprocesssing Dataset
We are taking the average of the motion data at every second and assigning corresponding sleep labels to each second.

### Tensorflow Model
The model we are running is a simple NN with one hidden layer. 

### Buidling 