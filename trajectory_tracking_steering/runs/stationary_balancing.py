from core.bike_sim_setup import setup_bike_sim  
kit, stage, prim_path = setup_bike_sim()  

from core.joint_setup import configure_bike_joint, configure_arm_joint
from core.bike_physics_config import dt, v_x, K1, K2, K3, K4

from omni.isaac.core.articulations import Articulation
from pxr import Gf, Sdf, UsdPhysics, UsdGeom
import omni.timeline
import math as m
import numpy as np
import matplotlib.pyplot as plt
import time
import random
from omni.isaac.core.utils.rotations import quat_to_euler_angles
from omni.isaac.core.objects import VisualSphere
from omni.isaac.core.simulation_context import SimulationContext

dt = 1/240
simulation_context = SimulationContext(physics_dt=dt, rendering_dt=1/240)
simulation_context.initialize_physics()

bike_joints = configure_bike_joint(stage)
arm_joints = configure_arm_joint(stage)

steering_drive = bike_joints["steering"]
rear_drive = bike_joints["rear"]

bike_joints = configure_bike_joint(stage)
arm_joints = configure_arm_joint(stage)

steering_drive = bike_joints["steering"]
rear_drive = bike_joints["rear"]

# omni.timeline.get_timeline_interface().play()
# kit.update()
art = Articulation(prim_path)
art.initialize()

def compute_roll_from_local_x():
    position, orientation = art.get_local_pose()
    rpy = quat_to_euler_angles(orientation)
    roll_angle = m.degrees(rpy[0])
    return roll_angle

steering_filtered = 0.0  # 초기 필터 출력값
T = 0.3                  # 시간 상수 (필터 강도 조절용, 필요시 튜닝)
def apply_lowpass_filter(steering_input, prev_filtered):
    alpha = dt / (T + dt)
    return alpha * steering_input + (1 - alpha) * prev_filtered

# 시뮬레이션 GUI 유지
print(f"[INFO] Imported bike model at {prim_path}")
print("[INFO] Close the window to end simulation.")


g = 9.81
mass = 34.9 #19.472
I_x = 1.8#0.15
h = 0.27#0.3201
R = 0.15#0.181
b = 0.34#0.3763
l = 0.74#0.70836
l_t = 0.05#0.0105 #0.0105
jeta = 31#13.5
K1 = 20
K2 = 0.5


roll_desired = 0
roll_current = 0
roll_past = 0
rear_rad_s = m.degrees((v_x/0.181))

log_steering = []
log_roll_dot = []
log_roll = []

for frame in range(10000):
    roll_current = compute_roll_from_local_x()
    roll_dot = (roll_current - roll_past) / dt
    f1 = mass*g*m.sin(m.radians(roll_current))/(I_x+mass*h**2)*(h+0.05*R*b/l)
    f2 = mass*g*b*m.cos(m.radians(roll_current))/(I_x+mass*h**2)/l*(l_t*m.cos(m.radians(jeta))-12*R*m.radians(jeta)/m.pi**2*(1-3**(1/2)/2))
    s = roll_current + K2*roll_dot
    print(f"roll_dot={roll_dot:.3f}, f1={f1:.3f}, s={s:.3f}, f2={f2:.3f}")
    steering_g = (-roll_dot-K2*f1-K1*s)/(K2*f2)
    #print(steering_g)
    steering_angle = m.degrees(m.atan2(m.tan(m.radians(steering_g))*m.cos(m.radians(roll_current)), m.cos(m.radians(jeta))))
    #print(steering_angle)
    steering_filtered = apply_lowpass_filter(steering_angle, steering_filtered)
    #print(roll_dot)
    steering_drive.GetTargetPositionAttr().Set(steering_filtered)
    rear_drive.GetTargetVelocityAttr().Set(0)
    roll_past = roll_current
    log_steering.append(steering_filtered)
    log_roll.append(roll_current)
    log_roll_dot.append(roll_dot)
    simulation_context.step(render=True)
    if roll_current > 90 or roll_current < -90:
        print("Roll angle out of bounds, stopping simulation.")
        break
# Shutdown and exit

t = np.linspace(0, dt * len(log_roll), len(log_roll))

plt.plot(t, log_roll, label='roll (deg)')
plt.plot(t, log_steering, label='steering (deg)')
plt.plot(t, log_roll_dot, label='roll dot (deg/s)')
plt.axhline(y=roll_desired, color='r', linestyle='--', label='desired roll')
plt.xlabel("Time (s)")
plt.legend()
plt.grid()
plt.title("Roll")
plt.ylim(-200, 200)  # 여기서 y축 범위 제한
plt.grid()
plt.legend()

# 화면에 띄우는 대신 파일로 저장
plt.savefig(f"/home/user404/Desktop/plot_stationary_data.png")
print("파일 저장 완료")
time.sleep(1)
#omni.timeline.get_timeline_interface().stop()
simulation_context.stop()
kit.close()