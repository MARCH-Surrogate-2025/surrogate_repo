from isaacsim import SimulationApp
kit = SimulationApp({"headless": False}) # GUI 없이 하려면 True

import omni.kit.commands  #URDF import 하기 위한 모듈
from pxr import Gf, Sdf, UsdPhysics, PhysxSchema, UsdLux  #순서대로 벡터, stage경로 정의, 물리엔진설정, 물리엔진설정, 광원
import omni.usd # 현재 sim에 열린 usd 가져오는 모듈

# 기본 URDF 설정 및 import
def setup_bike_sim(urdf_path="/home/user404/Documents/Isaac/URDF/ASSY_BIKE14_ARMED/urdf/ASSY_BIKE14_ARMED.urdf"):
    # URDF Import
    status, import_config = omni.kit.commands.execute("URDFCreateImportConfig")
    import_config.fix_base = False
    import_config.merge_fixed_joints = False
    import_config.convex_decomp = False
    import_config.import_inertia_tensor = True
    import_config.distance_scale = 1.0

    # prim_path : 최상위 루트
    status, prim_path = omni.kit.commands.execute(
        "URDFParseAndImportFile",
        urdf_path=urdf_path,
        import_config=import_config,
        get_articulation_root=True,
    )

    # 현재 stage 받아옴
    stage = omni.usd.get_context().get_stage()

    # gravity 설정
    scene = UsdPhysics.Scene.Define(stage, Sdf.Path("/physicsScene"))
    scene.CreateGravityDirectionAttr().Set(Gf.Vec3f(0, 0, -1))
    scene.CreateGravityMagnitudeAttr().Set(9.81)

    # 물리엔진 활성화
    PhysxSchema.PhysxSceneAPI.Apply(stage.GetPrimAtPath("/physicsScene"))
    physxSceneAPI = PhysxSchema.PhysxSceneAPI.Get(stage, "/physicsScene")
    physxSceneAPI.CreateEnableCCDAttr(True)
    physxSceneAPI.CreateEnableStabilizationAttr(True)
    physxSceneAPI.CreateEnableGPUDynamicsAttr(False)
    physxSceneAPI.CreateBroadphaseTypeAttr("MBP")
    physxSceneAPI.CreateSolverTypeAttr("TGS")

    # Ground + Light
    omni.kit.commands.execute(
        "AddGroundPlaneCommand",
        stage=stage,
        planePath="/groundPlane",
        axis="Z",
        size=1500.0,
        position=Gf.Vec3f(0, 0, -0.0589-0.17695),
        color=Gf.Vec3f(0.5),
    )
    UsdLux.DistantLight.Define(stage, Sdf.Path("/DistantLight")).CreateIntensityAttr(500)

    return kit, stage, prim_path  # 외부에서 사용할 수 있게 반환

