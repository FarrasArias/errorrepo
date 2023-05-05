from transformers import Trainer, TrainingArguments

import os
import json
import time
import torch
#from torch.utils.data import Dataset

import datetime
import numpy as np

import mmm_refactored as mmm

class CustomDataset:
  def __init__(self, split_id=0, is_training=True, batch_size=32, dataset=None, num_bars=4, min_tracks=2, max_tracks=12, max_seq_len=2048, opz=False, no_max_length=False, encoding=None, pad_value=-100, arch="gpt2", accum_steps=1, batches_per_epoch=1000, overload_batches_per_epoch=None, **kwargs):
    # settings
    self.is_training = is_training
    self.batch_size = batch_size // accum_steps
    self.split_id = split_id
    self.max_seq_len = max_seq_len
    self.batches_per_epoch = batches_per_epoch if overload_batches_per_epoch is None else overload_batches_per_epoch
    self.dataset = list(range(self.batches_per_epoch)) # number of examples ??
    self.pad_value = pad_value
    self.arch = arch

    # create dataloader
    self.dataloader = mmm.Jagged(dataset)
    self.dataloader.set_num_bars(num_bars)
    self.dataloader.set_min_tracks(min_tracks)
    self.dataloader.set_max_tracks(max_tracks)
    self.dataloader.set_max_seq_len(max_seq_len)
    seed = np.random.randint(2**20)
    self.dataloader.set_seed(seed)
    self.encoder_mode = mmm.getEncoderType(encoding)
    
    # create train_config
    self.tc = mmm.TrainConfig()
    self.tc.num_bars = num_bars
    self.tc.min_tracks = min_tracks
    self.tc.max_tracks = max_tracks
    self.tc.opz = opz
    self.tc.no_max_length = no_max_length

    self.current = 0
  
  def _get_batch(self):
    
    print("BLOOOOOKS1")
    input_ids, mask = self.dataloader.read_batch_v2(
      self.batch_size, self.split_id, self.encoder_mode, self.tc)
    print("BLOOOOOKS2")
    input_ids = np.array(input_ids)
    print("BLOOOOOKS3")
    mask = np.array(mask)
    print("BLOOOOOKS4")
    labels = np.copy(input_ids)
    labels += (1-mask) * self.pad_value # set masked tokens to pad_value
    batch = {
      "input_ids" : torch.from_numpy(input_ids), 
      "attention_mask" : torch.from_numpy(mask),
      "labels" : torch.from_numpy(labels)
    }
    if self.arch == "xl":
      batch.pop("attention_mask")
      assert np.all(np.sum(mask,axis=1)==self.max_seq_len)
    return batch
  
  def _get_batch_test(self):
    inputs = torch.ones((32,800), dtype=torch.int64)
    return {
      "input_ids" : inputs,
      "labels" : inputs
    }

  def __iter__(self):
      self.current = 0
      return self

  def __next__(self):
    self.current += 1
    if self.current <= self.batches_per_epoch:
      while True:
        try:
          return self._get_batch()
        except Exception as e:
          print("ERROR IN BATCHER : ", e)
    raise StopIteration
  
  def __len__(self):
    return self.batches_per_epoch

def pad(seqs, pad_value):
  seqlens = np.array([len(seq) for seq in seqs])
  maxlen = np.max(seqlens)
  return np.array([np.pad(seq, (0,maxlen-len(seq)), mode="constant", constant_values=pad_value) for seq in seqs]), seqlens

class FeatureDataset(CustomDataset):
  def _get_batch(self):
    input_ids, mask, features = self.dataloader.read_batch_w_feature(
      self.batch_size, self.split_id, self.encoder_mode, self.tc)
    input_ids = np.array(input_ids)
    mask = np.array(mask)
    labels = np.copy(input_ids)
    labels += (1-mask) * self.pad_value # set masked tokens to pad_value
    features = np.array(features)
    batch = {
      "input_ids" : torch.from_numpy(input_ids), 
      "attention_mask" : torch.from_numpy(mask),
      "labels" : torch.from_numpy(labels),
      "control_ids" : torch.from_numpy(features).float(),
    }
    print({k:v.shape for k,v in batch.items()})
    return batch

class EncoderDataset(CustomDataset):
  def _get_batch(self):
    inputs = {"x_p" : [], "y_p" : [], "x_n" : [], "y_n" : []}
    a, b = self.dataloader.load_piece_pair_batch(
      self.batch_size*3, self.split_id, self.encoder_mode, self.tc)
    split = np.random.rand(self.batch_size*3) < .5
    B = self.batch_size
    inputs = {
      "x_p" : [a[i] if split[i] else b[i] for i in range(B)],
      "y_p" : [b[i] if split[i] else a[i] for i in range(B)],
      "x_n" : [a[i] if split[i] else b[i] for i in range(B,2*B)],
      "y_n" : [a[i] if split[i] else b[i] for i in range(2*B,3*B)]
    }    
    # this is the one pass version
    torch_inputs = {}
    padded, seqlens = pad(
      inputs["x_p"] + inputs["x_n"] + inputs["y_p"] + inputs["y_n"],0)
    attention_mask = np.arange(padded.shape[1])[None,:] < seqlens[:,None]

    # under some conditions don't pass this on
    if padded.shape[1] >= 1024:
      raise RuntimeError("BATCH TOO BIG ({})".format(padded.shape[1]))

    torch_inputs["input_ids"] = torch.from_numpy(padded)
    torch_inputs["sequence_lengths"] = torch.from_numpy(seqlens)
    torch_inputs["attention_mask"] = torch.from_numpy(attention_mask)
    return torch_inputs

    # this is the multi pass version
    torch_inputs = {}
    for k,v in inputs.items():
      padded, seqlens = pad(v, self.pad_value)
      attention_mask = np.arange(padded.shape[1])[None,:] < seqlens[:,None]
      torch_inputs[k] = torch.from_numpy(padded)
      torch_inputs[k + "_sl"] = torch.from_numpy(seqlens)
      torch_inputs[k + "_am"] = torch.from_numpy(attention_mask)
    return torch_inputs

if __name__ == "__main__":

  import argparse
  parser = argparse.ArgumentParser()
  parser.add_argument("--arch", type=str, required=True)
  parser.add_argument("--config", type=str, required=True)
  parser.add_argument("--encoding", type=str, required=True)
  parser.add_argument("--dataset", type=str, required=True)
  parser.add_argument("--pad_value", type=int, default=-100)

  parser.add_argument("--opz", action="store_true")
  parser.add_argument("--num_bars", type=int, default=4)
  parser.add_argument("--min_tracks", type=int, default=2)
  parser.add_argument("--max_tracks", type=int, default=12)
  parser.add_argument("--max_seq_len", type=int, default=2048)

  parser.add_argument("--ngpu", type=int, default=4)
  parser.add_argument("--accum_steps", type=int, default=1)
  parser.add_argument("--batch_size", type=int, default=32)
  parser.add_argument("--lr", type=float, default=1e-4)

  parser.add_argument("--overwrite", type=int, default=1)
  parser.add_argument("--save_steps", type=int, default=5000)
  parser.add_argument("--log_steps", type=int, default=100)
  parser.add_argument("--step", type=int, default=0)
  parser.add_argument("--label", type=str, default="version3")

  args = parser.parse_args()

  dataset = CustomDataset(split_id=0, is_training=True, **vars(args))

  count = 0
  for batch in dataset:
    print("loaded")
    count += 1
    if count == 10:
      break
