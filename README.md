[![N|Solid](https://drive.google.com/uc?export=view&id=1u4xiWN3s0PAii8zn3-qxJ7wn35tBOypY)](https://metacreation.net/category/projects/)

# MMM Refactored Guide

If you're unfamiliar with Compute Canada, make sure to check the introductory .md [here]().

## mmm_api Installation - Cedar and Niagara
0. You might want to allocate an interactive session with salloc:

>**Note:** You DON'T need to do this in Niagara.

```sh
salloc --time=3:0:0 --nodes 1 --cpus-per-task 32 --mem=128000 --account=def-pasquier
```

1. First, make sure to clone the MMM_API into a folder in your CC machine:
```sh
https://<TOKENNUMBERS>@github.com/FarrasArias/mmm_refactored.git
```
2. Then we must load the standard environments and some dependencies:

>**Note:** If you're building in Niagara, load this first:
```sh
module load CCEnv arch/avx512
```
Then proceed to load the rest (If you're in Cedar, start from here):
```sh
module load StdEnv/2020
module load cmake/3.23.1
module load gcc/11.3.0
module load protobuf/3.12.3
module load python/3.8.2
```
3. Then we must create an environment and activate it:
```sh
virtualenv --no-download ./ENV                # ENV is the name of the environment
source ./ENV/bin/activate
pip install --no-index --upgrade pip          
```
4. Finally, just call the bash script with the correct argument:
```sh
bash create_python_library.sh –test_build –compute_canada
```
> **Note:** If you run the code without the --test_build flag, it will still compile and create the python library but it won't test it with the current model in production.
> **Note:** The other flag (--compute_canada) is necesary to build the code properly.

That's it!

Everything should get installed correctly in your python environment! If you log out and back in to CC make sure to activate the environment in which you installed the mmm_api.

## MMM Training

### Dataset Building

In order to train a new model, you must first build a dataset. You can upload the files you need using Globus (check the CC [guide]()). To check an example of the structure of the folders in your dataset, you can check the DDD_split.zip file in the shared folder (projects/def-pasquier). 
> **Note**: Remember that to copy from the shared folder to your own folders you must use absolute paths.

Once you have the folder with the data, run the following command
```sh
To be update Monday 30/04/2023
```

### Training a Model (TO BE UPDATE MONDAY 30/04/2023)
First, download the MMM_TRAINING project [here](https://gitlab.com/jeffreyjohnens/MMM_TRAINING/-/tree/master/). You can transfer it through Globus to CC or download it directly there with git.

To train a model, run the train.py file. Different lab members have managed to set the paths differently. What works for me is to use global paths. An example would be:
```sh
python train.py --arch gpt2 --config /home/raa60/scratch/MMM_TRAINING-master/config/gpt2_tiny.json --encoding EL_VELOCITY_DURATION_POLYPHONY_YELLOW_FIXED_ENCODER --ngpu 4 --dataset /home/raa60/scratch/farrastest_NUM_BARS=4_OPZ_False.arr --batch_size 32 --label DELETE_ME
```
Important things to look out for:
1. The choice of encoder matters. Right now, Jeff recommedns using EL_VELOCITY_DURATION_POLYPHONY_YELLOW_FIXED_ENCODER.

### Running Jobs

To read the CC documentation, cick [here](https://docs.alliancecan.ca/wiki/Running_jobs). You can run small snippets of code to test things out without allocating any resources. However, to train a model or perform any time/resource consuming task, you must schedule a job. A list of different types of job scheduling will be added here.

#### Interactive Jobs
You can start an interactive session on a compute node with salloc.
```sh
salloc --time=3:0:0 --nodes 1 --cpus-per-task 32 --mem=128000 --account=def-pasquier
```

#### Scheduled jobs (use this for training)
For time-expensive tasks it is better to create a bash file and submit a job with sbatch:
```sh
sbatch simple_job.sh
```

Here is an example of the contents of a bash file to submit a mmm training job:
```sh
#!/bin/bash
#SBATCH --gres=gpu:v100l:4
#SBATCH --cpus-per-task=32
#SBATCH --exclusive
#SBATCH --mem=0
#SBATCH --time=2-23:00
#SBATCH --account=def-pasquier
#SBATCH --mail-user USERNAME@sfu.ca  <---- MAKE SURE TO PUT YOUR EMAIL
#SBATCH --mail-type ALL
#SBATCH --output=CCLOG/FILENAME.out  <---- MAKE SURE TO CHANGE THE NAME OF THE FILE

source $SCRATCH/PY_3610/bin/activate   <---- THIS IS THE DIRECTORY TO THE ENV WHERE YOU HAVE THE mmm_api INSTALLED
cd $SCRATCH/MMM_TRAINING-master
module load StdEnv/2020 protobuf python/3.6.10
source $SCRATCH/PY_3610/bin/activate  <---- SAME HERE, MAKE SURE THE DIRECTORY IS PLACED CORRECTLY
python train.py --arch reformer --config /home/raa60/scratch/MMM_TRAINING-master/config/reformer.json --encoding EL_VELOCITY_DURATION_POLYPHONY_YELLOW_FIXED_ENCODER --ngpu 4 --dataset /home/raa60/scratch/dataset_NUM_BARS=4_OPZ_False.arr --batch_size 32 --label DELETE_ME
```

In this case we are using 4 v1001 GPUs (**gres** argument) and we're asking for 2 days and 23 hours of time to run the job (**time** argument).

#### Check jobs and eliminate session
To show all the users
```sh
who -u
```

To kill all the sessions
```sh
pkill -u username
```


