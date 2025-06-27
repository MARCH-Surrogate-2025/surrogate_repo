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

bike_joints = configure_bike_joint(stage)
arm_joints = configure_arm_joint(stage)

steering_drive = bike_joints["steering"]
rear_drive = bike_joints["rear"]

bike_joints = configure_bike_joint(stage)
arm_joints = configure_arm_joint(stage)

steering_drive = bike_joints["steering"]
rear_drive = bike_joints["rear"]

omni.timeline.get_timeline_interface().play()
kit.update()
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

dt = 1 / 60

a = 0.3763
b = 0.70836
c = 0.0105
h = 0.3201
HeadTubeAngle = 76.5
sinHeadTubeAngle = m.sin(m.radians(HeadTubeAngle))

v_x = (a * c * 9.81 * sinHeadTubeAngle / h) ** 0.5
v_x *= 0.5

K0 = -(h*v_x**2*sinHeadTubeAngle-a*c*9.81*sinHeadTubeAngle**2)/(9.81*b*h)
K2 = 50
K3 = -2
K1 = -K3*1.7

M1 = a * h * v_x * sinHeadTubeAngle
M2 = h * v_x ** 2 * sinHeadTubeAngle - a * c * 9.81 * sinHeadTubeAngle ** 2
M3 = b * h ** 2
M4 = b * 9.81 * h

roll_desired = 0
roll_current = 0
roll_past = 0
rear_rad_s = m.degrees((v_x/0.181))

log_steering = []

log_roll = []

for frame in range(10000):
    roll_current = compute_roll_from_local_x()
    roll_dot = (roll_current - roll_past) / dt
    steering_angle = K1*(roll_desired - roll_current) - K2*(roll_dot) + K3*roll_desired
    steering_filtered = apply_lowpass_filter(steering_angle, steering_filtered)
    steering_drive.GetTargetPositionAttr().Set(steering_filtered)
    #steering_drive.GetTargetPositionAttr().Set(steering_angle)
    rear_drive.GetTargetVelocityAttr().Set(rear_rad_s)
    roll_past = roll_current
    print(roll_past)
    print(v_x, K3)
    log_roll.append(roll_current)
    log_steering.append(steering_filtered)
    kit.update()
    # if roll_current > 90 or roll_current < -90:
    #     print("Roll angle out of bounds, stopping simulation.")
    #     break
# Shutdown and exit

t = np.linspace(0, dt * len(log_roll), len(log_roll))

plt.plot(t, log_roll, label='roll (deg)')
plt.plot(t, log_steering, label='steering (deg)')
plt.axhline(y=roll_desired, color='r', linestyle='--', label='desired roll')
plt.xlabel("Time (s)")
plt.legend()
plt.grid()
plt.title("Roll")
plt.ylim(-40, 40)  # 여기서 y축 범위 제한
plt.grid()
plt.legend()

# 화면에 띄우는 대신 파일로 저장
plt.savefig(f"/home/user404/Desktop/plot_roll_data_{roll_desired}degree.png")
print("파일 저장 완료")
time.sleep(1)
omni.timeline.get_timeline_interface().stop()
kit.close()
