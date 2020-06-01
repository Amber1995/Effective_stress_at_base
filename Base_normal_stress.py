#!/usr/bin/env python
# coding: utf-8

# In[12]:


import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from pylab import *
import os
import shutil
import subprocess
import glob

condition = ["dry"]
volume = ["1"]
a= "02"

#The initial time-step
ini_dat = "DEM000002.dat"


for c in condition:
    for v in volume:
        
        #path to the inputfiles(initial DEM*.dat) 
        input_path_ini = "/home/amber/Documents/2d-lbm-dem-analysis/2d-lbm-dem/Analysis/a02/"+c+"_v"+v+"00dat/"

        #path to the inputfiles(DEM*.dat at time-step corresponding to 3tau_c for different volumes) 
        input_path_3tc= "/home/amber/Documents/2d-lbm-dem-analysis/2d-lbm-dem/Analysis/a"+a+"/code/contactforces/"
     
        # List of all the files at 3tauc_c
        os.chdir(input_path_3tc)     
        files = filter(os.path.isfile, os.listdir('./')) 
        # Copy the corresponding DEM*.dat at 3 tau_c to output_path
        for doc in files:
            if doc.startswith(c+"_v"+v):
                file_3tc = shutil.copyfile(doc,output_path+c+"-v"+v+"_3tc.dat")   

        #path to the outpufiles and excutable files(current location)
        output_path = "/home/amber/Documents/2d-lbm-dem-analysis/2d-lbm-dem/Analysis/a"+a+"/code/contactforces/Using_g[i].stress/"


    # Process the initial DEM*.dat and obtain the maximum initial stress component at base of flowfront:smax

        # Copy the corresponding initial DEM*.dat to output_path
        filename = input_path_ini+ini_dat
        os.chdir(output_path)
        file_ini = shutil.copyfile(filename,"./"+c+"-v"+v+"_ini.dat")
        # Calculate the effective stress component of the grains at base of flowfront
        subprocess.call(['./voro-area.sh'])
        # Remove the inputfile to avoid out-of-order in following calculation
        os.remove(file_ini)
        # Obtain the maximum initial stress component at base of flowfront
        for file_ini_stress in glob.glob('./*stress.txt'):
            data_ini = pd.read_csv(file_ini_stress,sep=',')
            smax = data_ini['S22'].max()

# Process the DEM*.dat at 3 tau_c 
# Obtain normalized length by ~15d versus normalized effective stress s22 by smax

        # Calculate the effective stress component of the grains at base of flowfront 
        output = c+"-v"+v+"base.txt"
        f1 = open(output, "w")
 
        subprocess.call(['./voro-area.sh'])
        os.remove(c+"-v"+v+"_3tc.dat")
        for file_3tc_stress in glob.glob('./*stress.txt'):
            data_3tc = pd.read_csv(file_3tc_stress,sep=',')
            for index, row in data_3tc.iterrows():
                f1.write("%f %f \n"% (row[1],row[6]/smax))    
        f1.close()
        #Sort those outputfiles by value
        subprocess.call(['./sort.sh'])

        


# In[ ]:





# In[ ]:




