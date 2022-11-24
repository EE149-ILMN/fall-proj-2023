# fall-proj-2023

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