from faster_whisper import WhisperModel
import sys
import pprint
import json

if len(sys.argv)<2: 
    print("Argument mising : file.mp3 output.json ")
    quit(1)


model_size = "large-v3"

# Run on GPU with FP16
#model = WhisperModel(model_size, device="cuda", compute_type="float16")
model = WhisperModel(model_size, device="cpu", compute_type="int8")

# or run on GPU with INT8
# model = WhisperModel(model_size, device="cuda", compute_type="int8_float16")
# or run on CPU with INT8
# model = WhisperModel(model_size, device="cpu", compute_type="int8")

segments, info = model.transcribe(sys.argv[1], beam_size=2,word_timestamps=True)

#print("Detected language '%s' with probability %f" % (info.language, info.language_probability))

result = []
for segment in segments :
    tmp = {"start": segment.start,"end":segment.end, "text":  segment.text , "words": [] }
    for w in segment.words:
       tmp["words"].append({"word": w.word,"start":w.start,"end":w.end})
    result.append(tmp)
    #print("[%.2fs -> %.2fs] %s" % (segment.start, segment.end, segment.text))

with open(sys.argv[2], 'w') as f:
    json.dump(result, f, indent= 4)
