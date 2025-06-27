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
rear_drive.GetStiffnessAttr().Set(3)
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
ARM1.GetTargetPositionAttr().Set(0)
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


def compute_roll_from_local_x():
    position, orientation = art.get_local_pose()
    rpy = quat_to_euler_angles(orientation)
    roll_angle = m.degrees(rpy[0])
    print(position)
    return roll_angle, position[1]

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
K2 = 0.7
K3 = 1/K0
K1 = -K3*1.7
K4 = 12

M1 = a*h*v_x*sinHeadTubeAngle
M2 = h*v_x**2*sinHeadTubeAngle-a*c*9.81*sinHeadTubeAngle**2
M3 = b*h**2
M4 = b*9.81*h

roll_desired = 0
roll_current = 0
roll_past = 0
rear_rad_s = m.degrees((v_x/0.181))
y_current = 0

log_steering = []
log_y = []
log_roll = []

omni.timeline.get_timeline_interface().play()
kit.update()
art = Articulation(prim_path)
art.initialize()
for frame in range(100000):
    roll_current, y_current = compute_roll_from_local_x()
    roll_dot = (roll_current - roll_past) / dt
    steering_angle = K1*(roll_desired - roll_current) - K2*(roll_dot) + K3*roll_desired + K4 * y_current
    steering_filtered = apply_lowpass_filter(steering_angle, steering_filtered)
    steering_drive.GetTargetPositionAttr().Set(steering_filtered)
    #steering_drive.GetTargetPositionAttr().Set(steering_angle)
    rear_drive.GetTargetVelocityAttr().Set(rear_rad_s)
    roll_past = roll_current
    #print(roll_current)
    #print(v_x, y_current)
    log_roll.append(roll_current)
    log_steering.append(steering_filtered)
    log_y.append(y_current)
    kit.update()
# Shutdown and exit

t = np.linspace(0, dt * len(log_roll), len(log_roll))

plt.plot(t, log_roll, label='roll (deg)')
plt.plot(t, log_steering, label='steering (deg)')
plt.plot(t, log_y, label='y position (m)')
plt.axhline(y=roll_desired, color='r', linestyle='--', label='desired roll')
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
omni.timeline.get_timeline_interface().stop()
kit.close()
