from core.bike_sim_setup import setup_bike_sim  
kit, stage, prim_path = setup_bike_sim()  

from core.joint_setup import configure_bike_joint, configure_arm_joint
from core.bike_physics_config import dt, v_x, K1, K2, K3, K4

dt = 1/60
def compute_pose():
    IMUvalue = 0 # IMU 센싱값 넣어야함.
    roll_angle = IMUvalue
    return roll_angle

steering_filtered = 0.0  # 초기 필터 출력값
T = 0.3                  # 시간 상수 (필터 강도 조절용, 필요시 튜닝)
def apply_lowpass_filter(steering_input, prev_filtered):
    alpha = dt / (T + dt)
    return alpha * steering_input + (1 - alpha) * prev_filtered

roll_desired = 0
roll_current = 0
roll_past = 0

# log_steering = []
# log_roll = []

for frame in range(1000):
    roll_current = compute_pose()
    roll_dot = (roll_current - roll_past) / dt
    steering_angle = K1*(roll_desired - roll_current) - K2*(roll_dot) + K3*roll_desired
    steering_filtered = apply_lowpass_filter(steering_angle, steering_filtered)
    #steering_filtered 값 steering motor에 넣기 (degree)
    #v_x 값 후륜 모터에 넣기 (m/s)
    roll_past = roll_current
    # log_roll.append(roll_current)
    # log_steering.append(steering_filtered)
    # dt마다 for loop돌 수 있도록 설정