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

target_x = 0
target_y = 0
i = 0  # target point index

extra_points_x = []
extra_points_y = []
def set_goal(x_now, y_now, yaw_now):
    global target_x, target_y, i
    base_x = random.uniform(5,7)/2
    base_y = random.uniform(-3,3)/2

    angle_rad = m.radians(yaw_now)
    target_x = x_now + base_x * m.cos(angle_rad) - base_y * m.sin(angle_rad)
    target_y = y_now + base_x * m.sin(angle_rad) + base_y * m.cos(angle_rad)

    extra_points_x.append(target_x)
    extra_points_y.append(target_y)
    # 고유한 Prim 이름 생성
    prim_path = f"/World/marker_sphere_{i}"
    i += 1  # 다음 호출에 대비해 증가

    sphere_prim = UsdGeom.Sphere.Define(stage, prim_path)
    sphere_prim.AddTranslateOp().Set(Gf.Vec3f(target_x, target_y, 0.01))
    sphere_prim.AddScaleOp().Set(Gf.Vec3f(0.05, 0.05, 0.05))

def compute_pose():
    position, orientation = art.get_local_pose()
    rpy = quat_to_euler_angles(orientation)
    roll_angle = m.degrees(rpy[0])
    pitch_angle = m.degrees(rpy[1])
    yaw_angle = m.degrees(rpy[2])
    orientation = np.array([roll_angle, pitch_angle, yaw_angle])
    return position, orientation

steering_filtered = 0.0  # 초기 필터 출력값
T = 0.4                  # 시간 상수 (필터 강도 조절용, 필요시 튜닝)
def apply_lowpass_filter(steering_input, prev_filtered):
    alpha = dt / (T + dt)
    return alpha * steering_input + (1 - alpha) * prev_filtered

# 시뮬레이션 GUI 유지
print(f"[INFO] Imported bike model at {prim_path}")
print("[INFO] Close the window to end simulation.")

roll_desired = 0
roll_current = 0
roll_past = 0
rear_rad_s = m.degrees((v_x/0.181))
y_current = 0
position_difference = [target_x, target_y]

log_steering = []
log_x = []
log_y = []
log_roll = []

omni.timeline.get_timeline_interface().play()
kit.update()
art = Articulation(prim_path)
art.initialize()

j = 99
prim_path = f"/World/marker_sphere_{j}"

sphere_prim = UsdGeom.Sphere.Define(stage, prim_path)
sphere_prim.AddTranslateOp().Set(Gf.Vec3f(0, 0, 0.01))
sphere_prim.AddScaleOp().Set(Gf.Vec3f(0.05, 0.05, 0.05))
for frame in range(100000):
    current_position, current_orientation = compute_pose()
    roll_current = current_orientation[0]  # roll angle
    position_difference = [target_x, target_y] - current_position[0:2]  # x, y position difference
    abs_position_difference = np.linalg.norm(position_difference) 
    goal_yaw = m.degrees(m.atan2(position_difference[1], position_difference[0]))  # 목표 yaw 각도
    roll_dot = (roll_current - roll_past) / dt
    
    if current_orientation[2] - goal_yaw > 80 or current_orientation[2] - goal_yaw < -80:
        set_goal(current_position[0], current_position[1], current_orientation[2])
        if i ==20:
            break
        continue
    if abs_position_difference < 0.5**2:
        steering_angle = K1*(roll_desired - roll_current) - K2*(roll_dot) + K3*roll_desired + K4 * abs_position_difference ** 2 * (current_orientation[2] - goal_yaw)  # yaw 각도 오차를 이용한 steering angle 계산
    else:
        steering_angle = K1*(roll_desired - roll_current) - K2*(roll_dot) + K3*roll_desired + K4 * (current_orientation[2] - goal_yaw)  # yaw 각도 오차를 이용한 steering angle 계산
    steering_filtered = apply_lowpass_filter(steering_angle, steering_filtered)
    steering_drive.GetTargetPositionAttr().Set(steering_filtered)
    #steering_drive.GetTargetPositionAttr().Set(steering_angle)
    rear_drive.GetTargetVelocityAttr().Set(rear_rad_s)
    roll_past = roll_current
    log_roll.append(roll_current)
    log_steering.append(steering_filtered)
    log_x.append(current_position[0])
    log_y.append(current_position[1])
    kit.update()
    if roll_current > 90 or roll_current < -90:
        print("Roll angle out of bounds, stopping simulation.")
        break
# Shutdown and exit

t = np.linspace(0, dt * len(log_roll), len(log_roll))

plt.plot(t, log_roll, label='roll (deg)')
plt.plot(t, log_steering, label='steering (deg)')
plt.xlabel("Time (s)")
plt.legend()
plt.grid()
plt.title("Roll")
plt.ylim(-20, 20)  
plt.grid()
plt.legend()

# 화면에 띄우는 대신 파일로 저장
#plt.savefig(f"/home/user404/Desktop/plot_roll_data_{roll_desired}degree.png")
plt.savefig("/home/user404/Desktop/plot_linear.png")
print("파일 저장 완료")
time.sleep(1)

plt.figure()
plt.plot(log_x, log_y, marker='o', markersize = 1)
plt.scatter(extra_points_x, extra_points_y, color='red', marker='x', s=50, label='Special Points')

plt.xlabel("X Position")
plt.ylabel("Y Position")
plt.title("Trajectory in XY Plane")
plt.axis('equal')  # 비율 고정
plt.grid(True)
plt.savefig("/home/user404/Desktop/random_trajectory_xy.png") 
print("파일 저장 완료")
omni.timeline.get_timeline_interface().stop()
kit.close()
