import matplotlib.pyplot as plt
from scipy import stats
import math
import pandas as pd
import numpy as np
import sys
import seaborn as sns
import os

def all_data(prefix):
	data_float_psi = pd.read_csv('/home/josh/remoteProp/data/' + str(prefix + '_float_data.csv'), header=1)
	data_float_psi.insert(2, "Float Pressure (psia)", [x+14.7 for x in data_float_psi["Float Pressure (psig)"]])

	data_prop_psi = pd.read_csv('/home/josh/remoteProp/data/' + str(prefix + '_prop_data.csv'), header=1)
	data_prop_psi.insert(2, "Prop Pressure (psia)", [x+14.7 for x in data_prop_psi["Prop Pressure (psig)"]])

	data_weight = pd.read_csv('/home/josh/remoteProp/data/' + str(prefix + '_loadcell_data.csv'), header=1)
	data_weight.insert(2, "Thrust (mN)", [x*9.81 for x in data_weight["Weight (?)"]])

	data_temp = pd.read_csv('/home/josh/remoteProp/data/' + str(prefix + '_temp_data.csv'), header=1)
	data_temp.insert(2, "Temperature (K)", [x+273.15 for x in data_temp["Exit Temperature (Celsius)"]])

	return [data_float_psi, data_prop_psi, data_weight, data_temp]


test_nos = ["20191204_143449", "20191204_143735", "20191204_144142", "20191204_144715", "20191204_145400"]
# test_nos = ["20191204_143735"]

colors = []
fig1, ax1 = plt.subplots(figsize=(6.5, 4), dpi=90)
ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis

for trial, test in enumerate(test_nos):
	test_data = all_data(test)

	# ax1.plot(test_data[0]['Time (s)'], test_data[0]['Float Pressure (psia)'], 
	# 	color='#1f77b4', 
	# 	label='Upstream Pres.', 
	# 	marker='x', 
	# 	markersize='10', 
	# 	linestyle='-', 
	# 	linewidth='1')
	ax1.plot(test_data[1]['Time (s)'], test_data[1]['Prop Pressure (psia)'], 
		# color='#2ca02c', 
		label='Downstream Pres.', 
		marker='+', 
		markersize='10', 
		linestyle='--', 
		linewidth='1')

	# ax2.plot(test_data[2]['Time (s)'], test_data[2]['Thrust (mN)'], 
	# 	color='#ff7f0e', 
	# 	label='Thrust', 
	# 	marker='o',
	# 	fillstyle='none', 
	# 	linestyle='-', 
	# 	linewidth='1')

	# Blue: #1f77b4
	# Orange: #ff7f0e
	# Green: #2ca02c
ax1.set_xlabel('Time (s)', color='#413839')
ax1.set_ylabel('Pressure (psia)', color='#413839')
ax2.set_ylabel('Thrust (mN)', color='#413839')

# ax1.set_xlim([0, 14])

ax1.tick_params(colors='#413839')
ax1.grid(which='major', axis='both', linestyle='--')
box = ax1.get_position()
ax1.set_position([box.x0, box.y0 + box.height*0.1, box.width, box.height*0.9])

fig1.legend(loc='center', bbox_to_anchor=(0.5, 0.03), ncol=5, frameon=False )
# plt.title('Single Plenum Discharge Pressure and Thrust ({1} mm Nozzle)'.format(test_nos,0.6), y=1.03, color='#413839')

plt.show()