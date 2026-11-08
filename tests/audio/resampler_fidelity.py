"""Offline independent SciPy reference/quality comparison; no user audio published.
Run with numpy/scipy installed. Optional --music is a private 44.1k stereo s16le file.
The normal release gate remains dependency-free and tests the production C code.
"""
import argparse, ctypes, json, subprocess, time
from pathlib import Path
import numpy as np
from scipy import signal
root=Path(__file__).resolve().parents[2]
ap=argparse.ArgumentParser();ap.add_argument('--output',required=True);ap.add_argument('--music');args=ap.parse_args()
build=root/'build/fidelity';build.mkdir(parents=True,exist_ok=True)
subprocess.run(['cc','-O2','-shared','-fPIC','-Isrc/audio','tests/audio/resampler_probe.c','src/audio/pcm_ring.c','-o',str(build/'probe.so')],cwd=root,check=True)
lib=ctypes.CDLL(str(build/'probe.so'));lib.probe.argtypes=[ctypes.c_void_p,ctypes.c_uint,ctypes.c_void_p,ctypes.c_uint,ctypes.c_uint,ctypes.c_void_p]
def run(x,chunk=480):
    x=np.ascontiguousarray(x,dtype='<i2');n=(len(x)-1000)*160//147
    y=np.zeros((n,2),np.float32);read=ctypes.c_ulonglong();start=time.perf_counter()
    assert lib.probe(x.ctypes.data,len(x),y.ctypes.data,n,chunk,ctypes.byref(read))
    return y,read.value,time.perf_counter()-start
h=signal.firwin(160*256+1,1/160,window=('kaiser',12))
def reference(x,n):
    return signal.resample_poly(np.pad(x.astype(float)/32768,((31,0),(0,0)),mode='edge'),160,147,window=h)[:n]
def snr(y,ref):
    a=y[1000:-1000];b=ref[1000:-1000]
    return float(10*np.log10(np.mean(b*b)/np.mean((a-b)**2)))
N=45100;t=np.arange(N)/44100
result={'tones':[]}
for f in [100,1000,5000,10000,15000,18000,20000]:
    x=np.rint(20000*np.sin(2*np.pi*f*t)).astype('<i2');x=np.column_stack((x,-x))
    y,read,cost=run(x);ref=reference(x,len(y));score=snr(y,ref)
    n=np.arange(len(y))[1000:-1000];a=y[1000:-1000,0];phase=2*np.pi*f*(n/48000-31/44100)
    basis=np.column_stack((np.sin(phase),np.cos(phase)));fit=np.linalg.lstsq(basis,a,rcond=None)[0]
    residual=a-basis@fit
    result['tones'].append({'hz':f,'reference_snr_db':score,'gain_db':float(20*np.log10(np.linalg.norm(fit)/(20000/32768))), 'thdn_db':float(10*np.log10(np.mean(residual**2)/np.mean(a*a)))})
    assert score>65,(f,score)
# Independent call sizes and restarted stream must produce bit-identical output.
x=np.random.default_rng(6161).integers(-16000,16000,(N,2),dtype=np.int16)
y,read,cost=run(x);y2,r2,_=run(x,1);y3,r3,_=run(x,256)
assert np.array_equal(y,y2) and np.array_equal(y,y3) and read==r2==r3
assert read==len(y)*147//160+2
result['chunk_restart_exact']=True;result['input_frames_read']=read;result['output_frames']=len(y)
result['white_noise_reference_snr_db']=snr(y,reference(x,len(y)))
# Transition-band energy near input Nyquist is intentionally attenuated.
x=np.rint(signal.sosfilt(signal.butter(12,19000,fs=44100,output='sos'),x.astype(float),axis=0)).astype('<i2')
y,_,_=run(x);result['bandlimited_noise_reference_snr_db']=snr(y,reference(x,len(y)))
imp=np.zeros((N,2),dtype='<i2');imp[2000]=20000
y,_,_=run(imp);ref=reference(imp,len(y));result['impulse_reference_snr_db']=snr(y,ref);result['delay_input_frames']=31
# Same input benchmark; include feed/copy and ring initialization.
costs=[run(x)[2] for _ in range(15)]
result['host_seconds_per_audio_second_median']=float(np.median(costs)/(len(y)/48000))
result['host_seconds_per_audio_second_worst']=float(max(costs)/(len(y)/48000))
if args.music:
    music=np.fromfile(args.music,dtype='<i2').reshape(-1,2)
    y,_,_=run(music);result['private_music_reference_snr_db']=snr(y,reference(music,len(y)))
# A one-second steady segment has exact FFT bins at integer frequencies.
result['spectrum']=[]
for f in [1000,10000,18000,20000,21000,22000]:
    xx=np.rint(20000*np.sin(2*np.pi*f*np.arange(89200)/44100)).astype('<i2')
    yy,_,_=run(np.column_stack((xx,-xx)))
    a=yy[48000:96000,0]
    assert len(a)==48000
    spec=np.abs(np.fft.rfft(a));mask=np.ones(len(spec),bool);mask[f-1:f+2]=False
    result['spectrum'].append({'hz':f,'gain_db':float(20*np.log10(spec[f]*2/48000/(20000/32768))),
        'largest_other_bin_dbc':float(20*np.log10(spec[mask].max()/spec[f]))})
# Bounded alternative kernels, evaluated on the same analytic 20k signal.
n=np.arange(48000); t=n*147/160; idx=np.floor(t).astype(int); frac=t-idx
wave=lambda q:np.sin(2*np.pi*20000*q/44100)
error_snr=lambda a,b:float(10*np.log10(np.mean(b*b)/np.mean((a-b)**2)))
xm,x0,x1,x2=wave(idx-1),wave(idx),wave(idx+1),wave(idx+2)
linear=x0*(1-frac)+x1*frac
cubic=((-.5*xm+1.5*x0-1.5*x1+.5*x2)*frac+(xm-2.5*x0+2*x1-.5*x2))*frac**2+(-.5*xm+.5*x1)*frac+x0
result['alternatives_20k_snr_db']={'linear':error_snr(linear,wave(t)),'cubic':error_snr(cubic,wave(t))}
for taps in [32,48,64]:
    delay=taps//2-1; k=np.arange(taps); v=k[None,:]+frac[:,None]-delay
    w=np.where(np.abs(v)<taps/2,.42+.5*np.cos(np.pi*v/(taps/2))+.08*np.cos(2*np.pi*v/(taps/2)),0)
    c=np.sinc(v)*w;c/=c.sum(axis=1)[:,None]
    result['alternatives_20k_snr_db'][str(taps)+'tap']=error_snr((c*wave(idx[:,None]-k)).sum(axis=1),wave(t-delay))
Path(args.output).write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))
