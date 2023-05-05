#!/bin/bash

test_build=false
compute_canada=false
mac_os=false

# Parse arguments
for arg in "$@"
do
  case $arg in
    --test_build)
      test_build=true
      shift
      ;;
    --compute_canada)
      compute_canada=true
	  shift
	  ;;
    --mac_os)
      mac_os=true
      shift
      ;;
    *)
      shift
      ;;
  esac
done

# Set CMake flags based on command line arguments
cmake_flags=""
if $compute_canada; then
  cmake_flags="$cmake_flags -Dcompute_canada=ON"
fi
#cmake_flags="-DCMAKE_VERBOSE_MAKEFILE:BOOL=ON $cmake_flags" #delete this line if you don't want verbose makefile


# Code to check if libtorch and pybind11 are already downloaded

libtorch_path="libraries/libtorch"
pybind11_path="libraries/pybind11"
midifile_path="libraries/midifile"

#libtorch_url="https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-1.7.0%2Bcpu.zip"
#libtorch_url="https://download.pytorch.org/libtorch/cu118/libtorch-cxx11-abi-shared-with-deps-2.0.0%2Bcu118.zip"
libtorch_url="https://download.pytorch.org/libtorch/cpu/libtorch-shared-with-deps-1.7.0%2Bcpu.zip"
pybind11_url="https://github.com/pybind/pybind11.git"
midifile_url="https://github.com/craigsapp/midifile"

if $mac_os; then
  libtorch_url="https://download.pytorch.org/libtorch/cpu/libtorch-macos-2.0.0.zip"
fi

# Check if libtorch folder exists and is not empty
if [ ! -d "$libtorch_path" ] || [ -z "$(ls -A "$libtorch_path")" ]; then
    echo "libtorch folder does not exist or is empty. Downloading and extracting..."
    mkdir -p "$libtorch_path"
    curl -L "$libtorch_url" -o libtorch.zip
    unzip -q libtorch.zip -d libraries/
    rm libtorch.zip
    echo "libtorch downloaded and extracted."
else
    echo "libtorch folder exists and is not empty. No need to download."
fi

# Check if pybind11 folder exists and is not empty
if [ ! -d "$pybind11_path" ] || [ -z "$(ls -A "$pybind11_path")" ]; then
    echo "pybind11 folder does not exist or is empty. Cloning the repository..."
    mkdir -p libraries
    git clone "$pybind11_url" "$pybind11_path"
    echo "pybind11 downloaded."
else
    echo "pybind11 folder exists and is not empty. No need to download."
fi

# Check if midifile folder exists and is not empty
if [ ! -d "$midifile_path" ] || [ -z "$(ls -A "$midifile_path")" ]; then
	echo "midifile folder does not exist or is empty. Cloning the repository..."
	mkdir -p libraries
	git clone "$midifile_url" "$midifile_path"
	echo "midifile downloaded."
else
	echo "midifile folder exists and is not empty. No need to download."
fi


# Code to run if --test_build is true
if $test_build; then
  
    
    ORIGINAL_FILE="el_yellow_ts.pt"
    TARGET_FILE="model.pt"
    TARGET_FILE_AFTER_BUILD="./python_lib/model.pt"
    URL="http://vault.sfu.ca/index.php/s/Ff2j19IrKS5R969/download"

    if [ ! -f "$TARGET_FILE_AFTER_BUILD" ]; then
        echo "File '$TARGET_FILE' not found on build folder. Checking in root folder..."
    else
        mv ./python_lib/model.pt ./
    fi

    if [ ! -f "$TARGET_FILE" ]; then
        echo "File '$TARGET_FILE' not found. Downloading from $URL..."
        wget --content-disposition -O "$ORIGINAL_FILE" "$URL"
        if [ $? -eq 0 ]; then
            echo "File '$ORIGINAL_FILE' downloaded successfully."
            mv "$ORIGINAL_FILE" "$TARGET_FILE"
            if [ $? -eq 0 ]; then
                echo "File '$ORIGINAL_FILE' renamed to '$TARGET_FILE'."
            else
                echo "Error renaming file '$ORIGINAL_FILE' to '$TARGET_FILE'."
            fi
        else
            echo "Error downloading file '$ORIGINAL_FILE'."
        fi
    else
        echo "File '$TARGET_FILE' already exists. No need to download."
    fi


fi

# Middle section of the script to build the python library
rm -r ./python_lib
mkdir ./python_lib
rm -r ./libraries/protobuf/build
mkdir ./libraries/protobuf/build
cd ./python_lib
if $mac_os; then
  cmake $cmake_flags .. -Dmac_os=ON -DCMAKE_PREFIX_PATH=$(python3 -c 'import torch;print(torch.utils.cmake_prefix_path)')
else
  cmake $cmake_flags .. -DCMAKE_PREFIX_PATH=$(python3 -c 'import torch;print(torch.utils.cmake_prefix_path)')
fi
make
python3 -c "import mmm_refactored; print('mmm_refactored python library built successfully')"

# Code to run if --test_build is true
if $test_build; then

    cd ..
    mv ./model.pt ./python_lib/
    cp pythoninferencetest.py ./python_lib
    cp python_scripts_for_testing/mmmtest3.mid ./python_lib
    cp python_scripts_for_testing/pythontest.py ./python_lib
    cp python_scripts_for_testing/build_dataset_test.py ./python_lib
    cd ./python_lib
    python3 pythoninferencetest.py
    python3 pythontest.py
    python3 build_dataset_test.py   

fi
















