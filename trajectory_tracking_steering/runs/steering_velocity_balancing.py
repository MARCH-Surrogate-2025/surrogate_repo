from core.bike_sim_setup import setup_bike_sim  
kit, stage, prim_path = setup_bike_sim()  

from core.joint_setup import configure_bike_joint, configure_arm_joint
from core.bike_physics_config import dt, v_x, K1, K2, K3, K4

import cvxpy
from omni.isaac.core.articulations import Articulation
import omni.timeline
import math as m
import numpy as np
import matplotlib.pyplot as plt
import time
from omni.isaac.core.utils.rotations import quat_to_euler_angles

class PIDController:
    def __init__(self, Kp, Ki, Kd, N, dt):
        self.Kp = Kp
        self.Ki = Ki
        self.Kd = Kd
        self.N = N        # derivative filter cutoff freq
        self.dt = dt

        # 내부 상태 변수
        self.prev_error = 0.0
        self.integral = 0.0
        self.D_prev = 0.0

        # 필터 계수
        self.alpha = (dt * N) / (1 + dt * N)

    def compute(self, error):
        # P-term
        P = self.Kp * error

        # I-term
        self.integral += error * self.dt
        I = self.Ki * self.integral

        # D-term with filtering
        derivative = (error - self.prev_error) / self.dt
        D = self.alpha * self.Kd * derivative + (1 - self.alpha) * self.D_prev

        # 상태 업데이트
        self.prev_error = error
        self.D_prev = D

        # 제어 입력 계산
        u = P + I + D
        return u

pid = PIDController(Kp=-82.6193, Ki=-69.4433, Kd=-22.4138, N=234.4655, dt=1/60)

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
    return roll_angle, position[1]

roll_desired = 1
roll_current = 0
roll_past = 0
rear_rad_s = m.degrees((v_x/0.181))
y_current = 0

log_steering = []
log_y = []
log_roll = []
print(v_x)
for frame in range(1000):
    roll_current, y_current = compute_pose()
    error = (roll_desired-roll_current)
    input = pid.compute(error)
    steering_drive.GetTargetVelocityAttr().Set(-input)
    rear_drive.GetTargetVelocityAttr().Set(rear_rad_s)
    
    log_roll.append(roll_current)
    log_steering.append(input)
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
plt.ylim(-50, 50)  # 여기서 y축 범위 제한
plt.grid()
plt.legend()

#plt.savefig(f"/home/user404/Desktop/plot_roll_data_{roll_desired}degree.png")
plt.savefig("/home/user404/Desktop/plot_velocity_balancing.png")
print("파일 저장 완료")
time.sleep(1)
omni.timeline.get_timeline_interface().stop()
kit.close()
