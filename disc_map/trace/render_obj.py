#!/usr/bin/env python3
"""Quick top-down + 3/4 render of an OBJ (level floor-plan check). PIL only."""
import sys, math
from PIL import Image, ImageDraw

def load(path):
    V, F = [], []
    for ln in open(path):
        if ln.startswith("v "):
            _, x, y, z = ln.split()[:4]; V.append((float(x), float(y), float(z)))
        elif ln.startswith("f "):
            F.append([int(p.split("/")[0]) - 1 for p in ln.split()[1:]])
    return V, F

def render(path, out, yaw=0.0, pitch=0.0, W=700, H=700):
    V, F = load(path)
    xs=[v[0] for v in V]; ys=[v[1] for v in V]; zs=[v[2] for v in V]
    cx,cy,cz=(min(xs)+max(xs))/2,(min(ys)+max(ys))/2,(min(zs)+max(zs))/2
    span=max(max(xs)-min(xs), max(zs)-min(zs), max(ys)-min(ys), 1)
    s=0.82*min(W,H)/span
    cy_, sy_=math.cos(yaw),math.sin(yaw); cp,sp=math.cos(pitch),math.sin(pitch)
    P=[]
    for (x,y,z) in V:
        x-=cx; y-=cy; z-=cz
        x2=x*cy_+z*sy_; z2=-x*sy_+z*cy_
        y2=y*cp - z2*sp; z3=y*sp+z2*cp
        P.append((W/2+x2*s, H/2+y2*s, z3))
    img=Image.new("RGB",(W,H),(18,20,26)); d=ImageDraw.Draw(img)
    tris=[]
    for f in F:
        if len(f)<3: continue
        zc=sum(P[i][2] for i in f)/len(f); tris.append((zc,f))
    tris.sort(key=lambda t:t[0])
    zmin=min(P[i][2] for i in range(len(P))); zmax=max(P[i][2] for i in range(len(P))) or 1
    for zc,f in tris:
        poly=[(P[i][0],P[i][1]) for i in f]
        t=(zc-zmin)/(zmax-zmin+1e-6); c=int(60+150*t)
        d.polygon(poly, fill=(c,c,min(255,c+25)), outline=(30,30,38))
    img.save(out); print("wrote", out, f"({len(V)} v, {len(F)} f)")

if __name__=="__main__":
    p=sys.argv[1]
    render(p, p.replace(".obj","_topdown.png"), yaw=0.0, pitch=math.radians(90))  # top-down (X-Z)
    render(p, p.replace(".obj","_persp.png"), yaw=math.radians(35), pitch=math.radians(35))
