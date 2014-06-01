import qbs	
import "../../Eks/EksBuild" as Eks;

Eks.Library {
  name: "ShiftCore"
  toRoot: "../../"

  Depends { name: "EksCore" }

  Export {
    Depends { name: "cpp" }
    Depends { name: "EksCore" }

    cpp.includePaths: [ "./include" ]
  }
}
