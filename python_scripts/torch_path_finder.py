import os
import torch

print("1")
torch_package_path = os.path.dirname(torch.__file__)
print("2")
lib_path = os.path.join(torch_package_path, "..", "lib")
print("3")
lib_path = os.path.abspath(lib_path)

print(lib_path)
