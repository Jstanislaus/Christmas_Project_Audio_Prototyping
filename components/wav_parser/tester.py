import wave
import numpy as np

filename = "Test_Audio.wav"

with wave.open(filename, "rb") as wav:
    print("Channels:", wav.getnchannels())
    print("Sample rate:", wav.getframerate())
    print("Bit depth:", wav.getsampwidth() * 8)
    print("Number of samples:", wav.getnframes())

    raw = wav.readframes(wav.getnframes())
    samples = np.frombuffer(raw, dtype="<i2")

print("\nPCM statistics:")
print("Minimum:", samples.min())
print("Maximum:", samples.max())
print("RMS:", np.sqrt(np.mean(samples.astype(np.float64)**2)))
print("First 20 samples:", samples[:2000])