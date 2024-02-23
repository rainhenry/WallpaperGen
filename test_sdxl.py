#!/usr/bin/python3
from diffusers import DiffusionPipeline
import torch
import gc
gc.disable()
pipe = DiffusionPipeline.from_pretrained("./stable-diffusion-xl-base-1.0", use_safetensors=True, torch_dtype=torch.float32, variant="fp16")
pipe = pipe.to("cpu")
prompt = "Peach blossom trees in the desert"
images = pipe(prompt=prompt).images[0]
images.save("test.jpg")

