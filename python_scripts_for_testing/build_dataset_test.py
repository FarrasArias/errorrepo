import mmm_refactored as mmm
import json
tc = mmm.TrainConfig()
#encoder = mmm.TrackEncoder()
encoder = mmm.ElVelocityDurationPolyphonyYellowEncoder()
jag = mmm.BytesToFile('newdataset.arr')
bytes = encoder.midi_to_json_bytes('mmmtest3.mid',tc,'[]')
jag.append_bytes_to_file_stream(bytes,0)
jag.close()
print("midi_to_json_bytes works")
