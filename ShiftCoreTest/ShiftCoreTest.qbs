import "../../Eks/EksBuild" as Eks;

Eks.Test {
  name: "ShiftCoreTest"
  toRoot: "../../"
  
  Depends { name: "ShiftCore" }

  files: [ "*.h", "*.cpp", "*.qrc" ]
}
