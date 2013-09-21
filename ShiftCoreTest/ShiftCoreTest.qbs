import "../../Eks/EksBuild" as Eks;

Eks.Test {
  name: "ShiftCoreTest"
  toRoot: "../../"

  files: [ "*.h", "*.cpp" ]
}
