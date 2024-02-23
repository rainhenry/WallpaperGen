#!/usr/bin/python3
from diffusers import DiffusionPipeline
import torch
import gc

def pipe_init(model_id):
    gc.disable()
    pipe = DiffusionPipeline.from_pretrained(model_id, use_safetensors=True, torch_dtype=torch.float32, variant="fp16")
    pipe = pipe.to("cpu")
    return pipe

def wallpaper_gen(prompt, resolution, output_file, pipe):
    gc.disable()
    images = pipe(prompt=prompt, width=resolution, height=resolution).images[0]
    images.save(output_file)
    return None

