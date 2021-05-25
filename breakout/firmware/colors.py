#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors
import colorsys

def rgb_to_hsl(rgb):

    hls = np.empty_like(rgb)

    _ = colorsys.rgb_to_hls(rgb[0], rgb[1], rgb[2])

    hls[0] = _[0]
    hls[1] = _[1]
    hls[2] = _[2]

    return hls

def hsl_to_rgb(hls):

    rgb = np.empty_like(hls)

    _ = colorsys.hls_to_rgb(hls[0], hls[1], hls[2])

    rgb[0] = _[0]
    rgb[1] = _[1]
    rgb[2] = _[2]

    return rgb

# From: https://stackoverflow.com/questions/47222585/matplotlib-generic-colormap-from-tab10
def categorical_cmap(nc, nsc, cmap="tab10", continuous=False):
    if nc > plt.get_cmap(cmap).N:
        raise ValueError("Too many categories for colormap.")
    if continuous:
        ccolors = plt.get_cmap(cmap)(np.linspace(0,1,nc))
    else:
        ccolors = plt.get_cmap(cmap)(np.arange(nc, dtype=int))

    cols = np.zeros((nc*nsc, 3))

    for i, c in enumerate(ccolors):
        chsv = rgb_to_hsl(c[:3])
        arhsv = np.tile(chsv,nsc).reshape(nsc,3)
        arhsv[:,1] = np.linspace(1.0/nsc, chsv[1],nsc)
        #arhsv[:,2] = np.linspace(chsv[2],1,nsc)
        rgb = np.apply_along_axis(hsl_to_rgb, 1, arhsv)
        cols[i*nsc:(i+1)*nsc,:] = rgb       

    return (cols, matplotlib.colors.ListedColormap(cols))

#c1 = categorical_cmap(12, 16, cmap="Set3", continuous=True)[1]
#plt.scatter(np.arange(160),np.ones(160)+5, c=np.arange(160), s=180, cmap=c1)
#
c1 = categorical_cmap(10, 16, cmap="tab10", continuous=True)[1]
plt.scatter(np.arange(160),np.ones(160)+4, c=np.arange(160), s=180, cmap=c1)

#c1 = categorical_cmap(8, 16, cmap="Accent", continuous=True)
#plt.scatter(np.arange(160),np.ones(160)+3, c=np.arange(160), s=180, cmap=c1)
#
#c1 = categorical_cmap(9, 16, cmap="Pastel1", continuous=True)
#plt.scatter(np.arange(160),np.ones(160)+2, c=np.arange(160), s=180, cmap=c1)
#
#c1 = categorical_cmap(8, 16, cmap="Pastel2", continuous=True)
#plt.scatter(np.arange(160),np.ones(160)+1, c=np.arange(160), s=180, cmap=c1)
#
#c1 = categorical_cmap(9, 16, cmap="Set1", continuous=True)
#plt.scatter(np.arange(160),np.ones(160), c=np.arange(160), s=180, cmap=c1)
    

#ax = plt.gca()
#ax.set_facecolor('xkcd:black')
#
#plt.show()


#


#%% Create hex initialization file
cols = categorical_cmap(10, 16, cmap="tab10", continuous=True)[0]

# The brightness of the colors needs to be really really toned down to look good on Neopixels
cols_hsv = np.apply_along_axis(lambda c: colorsys.rgb_to_hsv(c[0], c[1], c[2]), 1, cols)
cols_hsv[:,1] = 0.98;
cols_hsv[:, 2] = 0.07 * cols_hsv[:, 2]
cols = np.apply_along_axis(lambda c: colorsys.hsv_to_rgb(c[0], c[1], c[2]), 1, cols_hsv)
cmap = matplotlib.colors.ListedColormap(cols)

# plt.close('all')
plt.figure()
plt.scatter(np.arange(160),np.ones(160)+4, c=np.arange(160), s=180, cmap=cmap)


#%%
cols = np.ceil(255 * cols)
cols = cols.astype('uint8') 

f = open(r"colors.mem","w")

for r in cols:
    for i, e in enumerate(r):
        
        if i == 2:
            f.write('%02x'%e + '\n')
        else:
            f.write('%02x'%e)
        
f.close()



