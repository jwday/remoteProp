import matplotlib.pyplot as plt
from scipy import stats
from scipy import signal
import math
import pandas as pd
import numpy as np
import sys
import seaborn as sns
import os

sns.set()
sns.axes_style("white")
sns.set_style("whitegrid", {"xtick.major.size": 0, "ytick.major.size": 0, 'grid.linestyle': '--'})
sns.set_context("paper", font_scale = 1, rc={"grid.linewidth": .5})
sns.set_palette(sns.color_palette("colorblind"))

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
# ---=== Steady-state Thrust Tests ===---
test_nos = {
	'100 psig': ["20191204_145400", "20191204_145419", "20191204_145442"],
	 '80 psig': ["20191204_144715", "20191204_144754", "20191204_144840"],
	 '60 psig': ["20191204_144142", "20191204_144207", "20191204_144256"],
	 '40 psig': ["20191204_143735", "20191204_143817", "20191204_143931"],
	 '20 psig': ["20191204_143449", "20191204_143538", "20191204_143636"]}

all_float_data = pd.DataFrame()
all_prop_data = pd.DataFrame()
all_thrust_data = pd.DataFrame()
all_data = pd.DataFrame()

for i, setpoint in enumerate(test_nos):
	for j, trial_no in enumerate(test_nos[setpoint]):
		prefix = trial_no
		data_float_psi = pd.read_csv('/home/josh/remoteProp/data/' + str(prefix + '_float_data.csv'), header=1)																# Load trial float pressure data
		resampled_n = int(round(data_float_psi['Time (s)'][-1:])/0.1)																										# Determine how many points are necessary to have even spacing at 0.1 sec
		resampled_time = np.linspace(0, resampled_n/10, resampled_n+1)																										# Make a new series of times based on the number of samples
		resampled_data = signal.resample(data_float_psi["Float Pressure (psig)"], resampled_n+1)																			# Resample the data
		data_float_psi_resampled = pd.DataFrame({'Time (s)': resampled_time, 'Pressure (psig)': resampled_data})															# Make a DataFrame object out of the resampled data
		data_float_psi_resampled = pd.concat([data_float_psi_resampled, pd.DataFrame([j+1 for x in data_float_psi_resampled.index], columns=['Trial'])], axis=1)			# Concatenate with column of Trial nos.
		data_float_psi_resampled = pd.concat([data_float_psi_resampled, pd.DataFrame([setpoint for x in data_float_psi_resampled.index], columns=['Set Point'])], axis=1)	# Concatenate with column of  Ps

		data_prop_psi = pd.read_csv('/home/josh/remoteProp/data/' + str(prefix + '_prop_data.csv'), header=1)
		resampled_n = int(round(data_prop_psi['Time (s)'][-1:])/0.1)																										# Determine how many points are necessary to have even spacing at 0.1 sec
		resampled_time = np.linspace(0, resampled_n/10, resampled_n+1)																										# Make a new series of times based on the number of samples
		resampled_data = signal.resample(data_prop_psi["Prop Pressure (psig)"], resampled_n+1)																				# Resample the data
		data_prop_psi_resampled = pd.DataFrame({'Time (s)': resampled_time, 'Pressure (psig)': resampled_data})																# Make a DataFrame object out of the resampled data
		data_prop_psi_resampled = pd.concat([data_prop_psi_resampled, pd.DataFrame([j+1 for x in data_prop_psi_resampled.index], columns=['Trial'])], axis=1)				# Concatenate with column of Trial nos.
		data_prop_psi_resampled = pd.concat([data_prop_psi_resampled, pd.DataFrame([setpoint for x in data_prop_psi_resampled.index], columns=['Set Point'])], axis=1)		# Concatenate with column of Setpoints

		data_thrust = pd.read_csv('/home/josh/remoteProp/data/' + str(prefix + '_loadcell_data.csv'), header=1)
		data_thrust.insert(2, "Thrust (mN)", [x*9.81 for x in data_thrust["Weight (?)"]])
		resampled_n = int(round(data_thrust['Time (s)'][-1:])/0.1)																											# Determine how many points are necessary to have even spacing at 0.1 sec
		resampled_time = np.linspace(0, resampled_n/10, resampled_n+1)																										# Make a new series of times based on the number of samples
		resampled_data = signal.resample(data_thrust["Thrust (mN)"], resampled_n+1)																							# Resample the data
		data_thrust_resampled = pd.DataFrame({'Time (s)': resampled_time, 'Thrust (mN)': resampled_data})																	# Make a DataFrame object out of the resampled data
		data_thrust_resampled = pd.concat([data_thrust_resampled, pd.DataFrame([j+1 for x in data_thrust_resampled.index], columns=['Trial'])], axis=1)						# Concatenate with column of Trial nos.
		data_thrust_resampled = pd.concat([data_thrust_resampled, pd.DataFrame([setpoint for x in data_thrust_resampled.index], columns=['Set Point'])], axis=1)				# Concatenate with column of Setpoints

		all_float_data = all_float_data.append(data_float_psi_resampled)
		all_prop_data  = all_prop_data.append(data_prop_psi_resampled)
		all_thrust_data = all_thrust_data.append(data_thrust_resampled)

fig1, axs = plt.subplots(2, sharex=True, dpi=300, figsize=(6, 4))
fig1.suptitle('Steady-State Pressure & Thrust Measurements', y=0.98, fontsize=12)
axs[0].set_title(r'CO$_2$ at $T_0=298$ K, Nozzle $\varnothing$0.6 mm, $\lambda$=1.17 (3$x$ Trials/Set Point)', fontsize=8, color='dimgray', y=1)

sns.lineplot(ax=axs[0], x="Time (s)", y="Pressure (psig)", hue="Set Point", style="Set Point", data=all_float_data)
sns.lineplot(ax=axs[1], x="Time (s)", y="Thrust (mN)", hue="Set Point", style="Set Point", data=all_thrust_data)

axs[0].set_ylabel(r'Pressure, $psig$', fontsize=8)
axs[1].set_ylabel(r'Thrust, $mN$', fontsize=8)

plt.xlim(left=0, right=16)

for ax in axs.flat:
		ax.legend(loc='upper right', fontsize=6, framealpha=0.9)
		
		ax.tick_params(axis='y', labelsize=6, pad=0)
		ax.yaxis.offsetText.set_fontsize(6)

		ax.tick_params(axis='x', labelsize=6, pad=0)
		ax.xaxis.label.set_size(8)
		ax.set(xlabel=r'Time, $sec$')

# plt.title('Single Plenum Discharge Pressure and Thrust ({1} mm Nozzle)'.format(test_nos,0.6), y=1.03, color='#413839')


# ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis

# for trial, test in enumerate(test_nos):
# 	test_data = all_data(test)

# 	ax1.plot(test_data[0]['Time (s)'], test_data[0]['Float Pressure (psig)'], 
# 		# color='#2ca02c', 
# 		label='Trial #' + str(trial+1) + ' Pres Raw.', 
# 		# marker='+', 
# 		markersize='10', 
# 		linestyle='--', 
# 		linewidth='1')
# 	ax1.plot(test_data[0]['Time Resampled (s)'], test_data[0]['Float Pressure Resampled (psig)'], 
# 		# color='#1f77b4', 
# 		label='Trial #' + str(trial+1) + ' Pres. Resampled', 
# 		# marker='x', 
# 		markersize='10', 
# 		linestyle='-', 
# 		linewidth='1')

# 	# ax2.plot(test_data[2]['Time (s)'], test_data[2]['Thrust (mN)'], 
# 	# 	# color='#ff7f0e', 
# 	# 	label='Trial #' + str(trial+1) + ' Thrust', 
# 	# 	marker='o',
# 	# 	fillstyle='none', 
# 	# 	linestyle='-', 
# 	# 	linewidth='1')		
# 	# ax2.plot(test_data[2]['Time Resampled (s)'], test_data[2]['Thrust Resampled (mN)'], 
# 	# 	# color='#ff7f0e', 
# 	# 	label='Trial #' + str(trial+1) + ' Thrust', 
# 	# 	marker='o',
# 	# 	fillstyle='none', 
# 	# 	linestyle='-', 
# 	# 	linewidth='1')

# 	# Blue: #1f77b4
# 	# Orange: #ff7f0e
# 	# Green: #2ca02c
# ax1.set_xlabel('Time (s)', color='#413839')
# ax1.set_ylabel('Pressure (psig)', color='#413839')
# ax2.set_ylabel('Thrust (mN)', color='#413839')

# ax1.set_xlim([0, 11])

# ax1.tick_params(colors='#413839')
# ax1.grid(which='major', axis='both', linestyle='--')
# box = ax1.get_position()
# ax1.set_position([box.x0, box.y0 + box.height*0.1, box.width, box.height*0.9])

# fig1.legend(loc='center', bbox_to_anchor=(0.5, 0.03), ncol=10, frameon=False )
# # plt.title('Single Plenum Discharge Pressure and Thrust ({1} mm Nozzle)'.format(test_nos,0.6), y=1.03, color='#413839')

# # plt.show()

# plt.close('all')
# linewidth = 2
# fontsize = 12
# fig2, ax1 = plt.subplots(figsize=(5.75, 4), dpi=200)

# td = []

# for trial, test in enumerate(test_nos):
# 	test_data = all_data(test)[0]  # All the Float Pressure data
# 	test_data["Trial"] = trial
# 	td.append(test_data)

# td = pd.concat(td)
# td['Time (s)'] = td['Time (s)'].round(1)
# sns.lineplot(ax=ax1, x=td['Time (s)'], y=td['Float Pressure (psia)'], data=td, label='Pressure (psia)', linewidth=linewidth)

# # plt.legend(loc='upper left', bbox_to_anchor=(0.68, 1), ncol=1, frameon=False )
# plt.legend(loc='center', bbox_to_anchor=(0.2, -0.25), ncol=1, frameon=False )

# plt.tick_params(colors='#413839')
# plt.grid(which='major', axis='both', linestyle='--')


# ax2 = plt.twinx()  # instantiate a second axes that shares the same x-axis
# td = []

# for trial, test in enumerate(test_nos):
# 	test_data = all_data(test)[2]  # All the Thrust data
# 	test_data["Trial"] = trial
# 	td.append(test_data)

# td = pd.concat(td)
# td['Time (s)'] = td['Time (s)'].round(1)
# sns.lineplot(x=td['Time (s)'], y=td['Thrust (mN)'], color='Orange', data=td, ax=ax2, label='Thrust (mN)', linewidth=linewidth)


# box = ax2.get_position()
# ax1.set_position([box.x0 + box.width * 0.03, box.y0 + box.height * 0.15, box.width * 0.95, box.height * 0.95])
# ax1.set_xlabel('Time, s', color='#413839', fontsize=fontsize)
# ax1.set_ylim
# ax1.set_ylabel('Pressure, psia', color='#413839', fontsize=fontsize)
# ax2.set_ylabel('Thrust, mN', color='#413839', fontsize=fontsize)

# # plt.legend(loc='upper left', bbox_to_anchor=(0.68, 0.95), ncol=1, frameon=False )
# plt.legend(loc='center', bbox_to_anchor=(0.7, -0.25), ncol=1, frameon=False )

# # plt.title('Supply Pressure and Thrust (0.6 mm Nozzle)')
# # plt.savefig('/mnt/d/OneDrive - UC Davis/HRVIP/Writing/AIAA SciTech 2019 Paper/Images/Sim Results/image.png')

plt.show()