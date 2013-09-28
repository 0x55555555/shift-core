import "../../Eks/EksBuild" as Eks;

Eks.Library {
  name: "ShiftCore"
  toRoot: "../../"

  property bool uiSupport: true

  Depends {
    name: "Eks.EksCore"
  }

  Properties {
    condition: uiSupport

    Depends { name: "Qt.gui" }
    Depends { name: "Qt.widgets" }
  }

  Export {
    Depends { name: "cpp" }
    Depends { name: "EksCore" }

    cpp.includePaths: [ "./include" ]
  }

  Depends {
    name: "ShiftCoreTest"
  }
}
