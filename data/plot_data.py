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


# Calculate water hammer pressure spike
dia_inner = 2.8/1000  # mm/1000 = m
area = math.pi*(dia_inner**2)/4  # m^2

temp_0 = -40  # C
temp_1 = -40.0385
m_dot = 0.76  # g/s
psia_0 = 113.2  # psia
psia_1 = 103  # psia

# ---------------------------------------------------------------
k = 1.298
R = 8.314/0.044041  # J/kg-K
temp_0 += 273.15
temp_1 += 273.15
m_dot /= 1000
pa_0 = psia_0*6894.76  # psia*6894.76 = Pascals (N/m^2)
pa_1 = psia_1*6894.76  # psia*6894.76 = Pascals (N/m^2)
rho_0 = pa_0/(R*temp_0)
rho_1 = pa_1/(R*temp_1)

a = math.sqrt(k*R*temp_0)

# Bernoulli says...
delta_v_bern = math.sqrt( 2*( (k*R/(k-1))*(temp_0 - temp_1) ) )
delta_v = m_dot/(rho_1*area)
hammer_pres = rho_0*a*delta_v  # Pascals (N/m^2)

print('')
print('dV Bernoulli: ' + str(delta_v_bern) + ' m/s')
print('dV from mdot: ' + str(delta_v) + ' m/s')
print('Speed of sound 0: ' + str(a) + ' m/s')
print('')
print("Hammer pressure (delta-P): " + str(hammer_pres/6894.76) + " psid")
print("Max pressure (psia): " + str((hammer_pres + pa_0)/6894.76) + " psia")
print('')
print('rho_0: ' + str(rho_0) + ' kg/m^3')
print('rho_1: ' + str(rho_1) + ' kg/m^3')
print('')

# Plot Data
# test_nos = ["20191205_191138",  "20191205_191433"]
# test_nos = ["20191205_191138",  "20191205_191210",  "20191205_191332",  "20191205_191402",  "20191205_191433"]
# test_nos = ["20191204_143735"]
test_nos = ["20191223_183908", "20191223_183945", "20191223_183832", "20191223_183725", "20191223_183658"]

colors = []
fig1, ax1 = plt.subplots(figsize=(6.5, 4), dpi=90)
ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis

for trial, test in enumerate(test_nos):
	test_data = all_data(test)

	ax1.plot(test_data[0]['Time (s)'], test_data[0]['Float Pressure (psia)'], 
		# color='#1f77b4', 
		label='Trial #' + str(trial+1) + ' Pres.', 
		marker='x', 
		markersize='10', 
		linestyle='-', 
		linewidth='1')
	# ax1.plot(test_data[1]['Time (s)'], test_data[1]['Prop Pressure (psia)'], 
	# 	# color='#2ca02c', 
	# 	label='Downstream Pres.', 
	# 	marker='+', 
	# 	markersize='10', 
	# 	linestyle='--', 
	# 	linewidth='1')

	ax2.plot(test_data[2]['Time (s)'], test_data[2]['Thrust (mN)'], 
		# color='#ff7f0e', 
		label='Trial #' + str(trial+1) + ' Thrust', 
		marker='o',
		fillstyle='none', 
		linestyle='-', 
		linewidth='1')

	# Blue: #1f77b4
	# Orange: #ff7f0e
	# Green: #2ca02c
ax1.set_xlabel('Time (s)', color='#413839')
ax1.set_ylabel('Pressure (psia)', color='#413839')
ax2.set_ylabel('Thrust (mN)', color='#413839')

ax1.set_xlim([0, 11])

ax1.tick_params(colors='#413839')
ax1.grid(which='major', axis='both', linestyle='--')
box = ax1.get_position()
ax1.set_position([box.x0, box.y0 + box.height*0.1, box.width, box.height*0.9])

fig1.legend(loc='center', bbox_to_anchor=(0.5, 0.03), ncol=10, frameon=False )
# plt.title('Single Plenum Discharge Pressure and Thrust ({1} mm Nozzle)'.format(test_nos,0.6), y=1.03, color='#413839')

# plt.show()

plt.close('all')
td = []

for trial, test in enumerate(test_nos):
	test_data = all_data(test)[0]
	test_data["Trial"] = trial
	td.append(test_data)

td = pd.concat(td)
td['Time (s)'] = td['Time (s)'].round(1)
sns.lineplot(x=td['Time (s)'], y=td['Float Pressure (psia)'], data=td, label='Pressure (psia)')

# plt.legend(loc='upper left', bbox_to_anchor=(0.68, 1), ncol=1, frameon=False )
plt.legend(loc='center', bbox_to_anchor=(0.3, -0.2), ncol=1, frameon=False )

plt.tick_params(colors='#413839')
plt.grid(which='major', axis='both', linestyle='--')




ax2 = plt.twinx()  # instantiate a second axes that shares the same x-axis
td = []

for trial, test in enumerate(test_nos):
	test_data = all_data(test)[2]
	test_data["Trial"] = trial
	td.append(test_data)

td = pd.concat(td)
td['Time (s)'] = td['Time (s)'].round(1)
sns.lineplot(x=td['Time (s)'], y=td['Thrust (mN)'], color='Orange', data=td, ax=ax2, label='Thrust (mN)')


box = ax2.get_position()
ax2.set_position([box.x0, box.y0 + box.height*0.1, box.width, box.height*0.9])

# plt.legend(loc='upper left', bbox_to_anchor=(0.68, 0.95), ncol=1, frameon=False )
plt.legend(loc='center', bbox_to_anchor=(0.6, -0.2), ncol=1, frameon=False )

plt.title('Supply Pressure and Thrust (0.6 mm Nozzle)')

plt.show()