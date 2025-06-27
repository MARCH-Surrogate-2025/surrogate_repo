from core.bike_sim_setup import setup_bike_sim  
kit, stage, prim_path = setup_bike_sim()  

from core.joint_setup import configure_bike_joint, configure_arm_joint
from core.bike_physics_config import dt, v_x, K1, K2, K3, K4

from omni.isaac.core.articulations import Articulation
import omni.timeline
import math as m
import numpy as np
import matplotlib.pyplot as plt
import time
from omni.isaac.core.utils.rotations import quat_to_euler_angles

bike_joints = configure_bike_joint(stage)
arm_joints = configure_arm_joint(stage)

steering_drive = bike_joints["steering"]
rear_drive = bike_joints["rear"]

omni.timeline.get_timeline_interface().play()
kit.update()
art = Articulation(prim_path)
art.initialize()

def compute_pose():
    position, orientation = art.get_local_pose()
    rpy = quat_to_euler_angles(orientation)
    roll_angle = m.degrees(rpy[0])
    print(position)
    print(v_x)
    return roll_angle, position[1]

steering_filtered = 0.0  # 초기 필터 출력값
T = 0.3                  # 시간 상수 (필터 강도 조절용, 필요시 튜닝)
def apply_lowpass_filter(steering_input, prev_filtered):
    alpha = dt / (T + dt)
    return alpha * steering_input + (1 - alpha) * prev_filtered

roll_desired = 0
roll_current = 0
roll_past = 0
rear_rad_s = m.degrees((v_x/0.181))
y_current = 0

log_steering = []
log_y = []
log_roll = []

for frame in range(1000):
    roll_current, y_current = compute_pose()
    roll_dot = (roll_current - roll_past) / dt
    steering_angle = K1*(roll_desired - roll_current) - K2*(roll_dot) + K3*roll_desired
    steering_filtered = apply_lowpass_filter(steering_angle, steering_filtered)
    steering_drive.GetTargetPositionAttr().Set(steering_filtered)
    rear_drive.GetTargetVelocityAttr().Set(rear_rad_s)
    roll_past = roll_current
    log_roll.append(roll_current)
    log_steering.append(steering_filtered)
    log_y.append(y_current)
    kit.update()
# Shutdown and exit

t = np.linspace(0, dt * len(log_roll), len(log_roll))
plt.plot(t, log_roll, label='roll (deg)')
plt.plot(t, log_steering, label='steering (deg)')
plt.plot(t, log_y, label='y (m)')
plt.axhline(y=roll_desired, color='r', linestyle='--', label='desired roll')
plt.xlabel("Time (s)")
plt.legend()
plt.grid()
plt.title("Roll")
plt.ylim(-10, 10)  # 여기서 y축 범위 제한
plt.grid()
plt.legend()

#plt.savefig(f"/home/user404/Desktop/plot_roll_data_{roll_desired}degree.png")
plt.savefig("/home/user404/Desktop/plot_balancing.png")
print("파일 저장 완료")
time.sleep(1)
omni.timeline.get_timeline_interface().stop()
kit.close()