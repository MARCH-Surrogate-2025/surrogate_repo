from isaacsim import SimulationApp
kit = SimulationApp({"headless": False})
import omni.kit.commands
from pxr import Gf, Sdf, UsdPhysics, PhysxSchema, UsdLux, UsdGeom, Usd
import omni.usd
import omni.timeline
from omni.isaac.core.articulations import Articulation
import math as m
from omni.isaac.core.utils.rotations import quat_to_euler_angles
import numpy as np
import matplotlib.pyplot as plt
import time
from omni.isaac.core.objects import VisualSphere
import random
# URDF import config
status, import_config = omni.kit.commands.execute("URDFCreateImportConfig")
import_config.fix_base = False
import_config.merge_fixed_joints = False
import_config.convex_decomp = False
import_config.import_inertia_tensor = True
import_config.distance_scale = 1.0

# URDF path
urdf_path = "/home/user404/Documents/Isaac/URDF/ASSY_BIKE14_ARMED/urdf/ASSY_BIKE14_ARMED.urdf"
status, prim_path = omni.kit.commands.execute(
    "URDFParseAndImportFile",
    urdf_path=urdf_path,
    import_config=import_config,
    get_articulation_root=True,
)

# Stage + physics
stage = omni.usd.get_context().get_stage()
scene = UsdPhysics.Scene.Define(stage, Sdf.Path("/physicsScene"))
scene.CreateGravityDirectionAttr().Set(Gf.Vec3f(0, 0, -1))
scene.CreateGravityMagnitudeAttr().Set(9.81)

PhysxSchema.PhysxSceneAPI.Apply(stage.GetPrimAtPath("/physicsScene"))
physxSceneAPI = PhysxSchema.PhysxSceneAPI.Get(stage, "/physicsScene")
physxSceneAPI.CreateEnableCCDAttr(True)
physxSceneAPI.CreateEnableStabilizationAttr(True)
physxSceneAPI.CreateEnableGPUDynamicsAttr(False)
physxSceneAPI.CreateBroadphaseTypeAttr("MBP")
physxSceneAPI.CreateSolverTypeAttr("TGS")

# ground & light
omni.kit.commands.execute(
    "AddGroundPlaneCommand",
    stage=stage,
    planePath="/groundPlane",
    axis="Z",
    size=1500.0,
    position=Gf.Vec3f(0, 0, -0.0589-0.17695),
    #position=Gf.Vec3f(0, 0, 0),
    color=Gf.Vec3f(0.5),
)

target_x = 0
target_y = 0
i = 0  # 고유한 Prim 이름을 위한 카운터

UsdLux.DistantLight.Define(stage, Sdf.Path("/DistantLight")).CreateIntensityAttr(500)

prim_path = "/ASSY_BIKE14_ARMED"  # Articulation Root


# Steering 각도 고정 (DEGREE → rad 아님!)
steering_prim = stage.GetPrimAtPath("/ASSY_BIKE14_ARMED/joints/JOINT_STEERING")
steering_drive = UsdPhysics.DriveAPI.Get(steering_prim, "angular")
if not steering_drive:
    steering_drive = UsdPhysics.DriveAPI.Apply(steering_prim, "angular")
steering_drive.GetTargetPositionAttr().Set(0)
#steering_drive.GetStiffnessAttr().Set(50000)
#steering_drive.GetDampingAttr().Set(5000)
#steering_drive.GetMaxForceAttr().Set(1000)


# DriveAPI 핸들 가져오기
rear_drive_prim = stage.GetPrimAtPath("/ASSY_BIKE14_ARMED/joints/JOINT_REAR_WHEEL")
rear_drive = UsdPhysics.DriveAPI.Get(rear_drive_prim, "angular")
if not rear_drive:
    rear_drive = UsdPhysics.DriveAPI.Apply(rear_drive_prim, "angular")
# 속도 제어 세팅 (deg/sec)
rear_drive.GetTargetVelocityAttr().Set(360)  # 360 deg/sec
rear_drive.GetStiffnessAttr().Set(0.1)
rear_drive.GetDampingAttr().Set(150)
rear_drive.GetMaxForceAttr().Set(100)

# DriveAPI 핸들 가져오기
front_drive_prim = stage.GetPrimAtPath("/ASSY_BIKE14_ARMED/joints/JOINT_FRONT_WHEEL")
front_drive = UsdPhysics.DriveAPI.Get(front_drive_prim, "angular")
if not front_drive:
   front_drive = UsdPhysics.DriveAPI.Apply(front_drive_prim, "angular")

#속도 제어 세팅 (deg/sec)
#front_drive.GetTargetVelocityAttr().Set(0)  # 360 deg/sec
front_drive.GetStiffnessAttr().Set(0)
front_drive.GetDampingAttr().Set(0)

ARM1_prim = stage.GetPrimAtPath("/ASSY_BIKE14_ARMED/joints/JOINT_ARM_1")
ARM1 = UsdPhysics.DriveAPI.Get(ARM1_prim, "angular")
if not ARM1:
    ARM1 = UsdPhysics.DriveAPI.Apply(ARM1_prim, "angular")
ARM1.GetTargetPositionAttr().Set(90)
ARM1.GetStiffnessAttr().Set(50)
ARM1.GetDampingAttr().Set(5)
ARM1.GetMaxForceAttr().Set(1000)

ARM2_prim = stage.GetPrimAtPath("/ASSY_BIKE14_ARMED/joints/JOINT_ARM_2")
ARM2 = UsdPhysics.DriveAPI.Get(ARM2_prim, "angular")
if not ARM2:
    ARM2 = UsdPhysics.DriveAPI.Apply(ARM2_prim, "angular")
ARM2.GetTargetPositionAttr().Set(0)
ARM2.GetStiffnessAttr().Set(50)
ARM2.GetDampingAttr().Set(5)
ARM2.GetMaxForceAttr().Set(1000)

ARM3_prim = stage.GetPrimAtPath("/ASSY_BIKE14_ARMED/joints/JOINT_ARM_3")
ARM3 = UsdPhysics.DriveAPI.Get(ARM3_prim, "angular")
if not ARM3:
    ARM3 = UsdPhysics.DriveAPI.Apply(ARM3_prim, "angular")
ARM3.GetTargetPositionAttr().Set(0)
ARM3.GetStiffnessAttr().Set(50)
ARM3.GetDampingAttr().Set(5)
ARM3.GetMaxForceAttr().Set(1000)

extra_points_x = []
extra_points_y = []

def set_goal(x_now, y_now, yaw_now):
    global target_x, target_y, i
    target_x = 3 * (i+1)
    target_y = 0

    extra_points_x.append(target_x)
    extra_points_y.append(target_y)
    # 고유한 Prim 이름 생성
    prim_path = f"/World/marker_sphere_{i}"
    i += 1  # 다음 호출에 대비해 증가

    sphere_prim = UsdGeom.Sphere.Define(stage, prim_path)
    sphere_prim.AddTranslateOp().Set(Gf.Vec3f(target_x, target_y, 0.01))
    sphere_prim.AddScaleOp().Set(Gf.Vec3f(0.05, 0.05, 0.05))

def compute_roll_from_local_x():
    position, orientation = art.get_local_pose()
    rpy = quat_to_euler_angles(orientation)
    roll_angle = m.degrees(rpy[0])
    pitch_angle = m.degrees(rpy[1])
    yaw_angle = m.degrees(rpy[2])
    orientation = np.array([roll_angle, pitch_angle, yaw_angle])
    return position, orientation

steering_filtered = 0.0  # 초기 필터 출력값
T = 0.3                  # 시간 상수 (필터 강도 조절용, 필요시 튜닝)
def apply_lowpass_filter(steering_input, prev_filtered):
    alpha = dt / (T + dt)
    return alpha * steering_input + (1 - alpha) * prev_filtered

# 시뮬레이션 GUI 유지
print(f"[INFO] Imported bike model at {prim_path}")
print("[INFO] Close the window to end simulation.")

#현재 dt = 1/60, 길이는 m, 각도는 deree
dt = 1/60
a = 0.3763
b = 0.70836
c = 0.0105
h = 0.3201
HeadTubeAngle = 76.5
sinHeadTubeAngle = m.sin(m.radians(HeadTubeAngle))
v_x = (a*c*9.81*sinHeadTubeAngle/h)**(1/2)
v_x = v_x * 3
K0 = -(h*v_x**2*sinHeadTubeAngle-a*c*9.81*sinHeadTubeAngle**2)/(9.81*b*h)
K2 = 0.5
K3 = 1/K0
K1 = -K3*1.5
K4 = 0.1

M1 = a*h*v_x*sinHeadTubeAngle
M2 = h*v_x**2*sinHeadTubeAngle-a*c*9.81*sinHeadTubeAngle**2
M3 = b*h**2
M4 = b*9.81*h

Kp = 0.2
Ki = 0.05
Kd = 0
prev_error = 0
error_sum = 0
past_position = [0,0,0]

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
log_vx = []
omni.timeline.get_timeline_interface().play()
kit.update()
art = Articulation(prim_path)
art.initialize()
for frame in range(100000):
    current_position, current_orientation = compute_roll_from_local_x()
    #print(current_position)
    if current_position is None or past_position is None:
        print("[WARNING] current_position or past_position is None!")
        print("current_position =", current_position)
        print("past_position =", past_position)
    else:
        dx = current_position[0] - past_position[0]
        dy = current_position[1] - past_position[1]
        #print(f"dx: {dx:.6f}, dy: {dy:.6f}")
        v_x_current = ((current_position[0] - past_position[0])**2 + (current_position[1] - past_position[1])**2)**0.5 / dt
        log_vx.append(v_x_current)
    past_position = current_position 
    error = v_x - v_x_current
    error_sum += error * dt
    error_diff = (error - prev_error) / dt
    cmd = Kp * error + Ki * error_sum + Kd * error_diff
    cmd = m.degrees((cmd / 0.181)) + rear_rad_s # Convert to degrees
    print(f"{rear_rad_s:.3f}, {cmd:.3f}, {v_x:.3f}, {v_x_current:.3f}")
    prev_error = error

    roll_current = current_orientation[0]  # roll angle
    position_difference = [target_x, target_y] - current_position[0:2]  # x, y position difference
    goal_yaw = m.degrees(m.atan2(position_difference[1], position_difference[0]))  # 목표 yaw 각도
    roll_dot = (roll_current - roll_past) / dt
    if current_orientation[2] - goal_yaw > 60 or current_orientation[2] - goal_yaw < -60:
        if i ==8:
            break
        set_goal(current_position[0], current_position[1], current_orientation[2])
        steering_angle = K1*(roll_desired - roll_current) - K2*(roll_dot) + K3*roll_desired
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
    #kit.update()
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
plt.ylim(-10, 10)  # 여기서 y축 범위 제한
plt.grid()
plt.legend()

# 화면에 띄우는 대신 파일로 저장
#plt.savefig(f"/home/user404/Desktop/plot_roll_data_{roll_desired}degree.png")
plt.savefig("/home/user404/Desktop/plot_linear.png")
print("파일 저장 완료")
time.sleep(1)

plt.figure()
plt.scatter(t, log_vx[1:], s=2, label='vx (m/s)')
print(len(t), len(log_vx))
plt.axhline(y=v_x, color='r', linestyle='--', label='desired vx')
plt.xlabel("Time (s)")
plt.legend()
plt.grid()
plt.title("vx")
plt.ylim(-2, 2)  # 여기서 y축 범위 제한
plt.grid()
plt.legend()
plt.savefig("/home/user404/Desktop/plot_vx.png")

plt.figure()
plt.plot(log_x, log_y, marker='o', markersize = 1)
plt.scatter(extra_points_x, extra_points_y, color='red', marker='x', s=50, label='Special Points')

plt.xlabel("X Position")
plt.ylabel("Y Position")
plt.title("Trajectory in XY Plane")
plt.axis('equal')  # 비율 고정
plt.grid(True)
plt.savefig("/home/user404/Desktop/linear_trajectory_xy.png") 
print("파일 저장 완료")

plt.savefig(f"/home/user404/Desktop/plot_roll_data_{roll_desired}degree.png")
omni.timeline.get_timeline_interface().stop()
kit.close()
