from pxr import UsdPhysics

def configure_bike_joint(stage):
    joints = {}

    def init_drive(path):
        prim = stage.GetPrimAtPath(path)
        drive = UsdPhysics.DriveAPI.Get(prim, "angular")
        return drive

    # 스티어링 조인트
    joints["steering"] = init_drive("/ASSY_BIKE14_ARMED/joints/JOINT_STEERING")
    #when position control
    joints["steering"].GetStiffnessAttr().Set(500)
    joints["steering"].GetDampingAttr().Set(10)
    #when velocity control
    #joints["steering"].GetStiffnessAttr().Set(0)
    #joints["steering"].GetDampingAttr().Set(150)
    joints["steering"].GetMaxForceAttr().Set(10)

    # 후륜 조인트
    joints["rear"] = init_drive("/ASSY_BIKE14_ARMED/joints/JOINT_REAR_WHEEL")
    joints["rear"].GetStiffnessAttr().Set(0)
    joints["rear"].GetDampingAttr().Set(150)
    joints["rear"].GetMaxForceAttr().Set(10)

    # 전륜 조인트
    joints["front"] = init_drive("/ASSY_BIKE14_ARMED/joints/JOINT_FRONT_WHEEL")
    joints["front"].GetStiffnessAttr().Set(0)
    joints["front"].GetDampingAttr().Set(0)
    joints["front"].GetMaxForceAttr().Set(10)
    return joints


def configure_arm_joint(stage):
    joints = {}

    def init_drive(path):
        prim = stage.GetPrimAtPath(path)
        drive = UsdPhysics.DriveAPI.Get(prim, "angular")
        return drive

    for i in range(1, 4):
        name = f"arm{i}"
        path = f"/ASSY_BIKE14_ARMED/joints/JOINT_ARM_{i}"
        drive = init_drive(path)
        drive.GetTargetPositionAttr().Set(0)
        drive.GetStiffnessAttr().Set(50)
        drive.GetDampingAttr().Set(5)
        drive.GetMaxForceAttr().Set(20)
        joints[name] = drive

    return joints
