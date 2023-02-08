import omni.usd
from usdrt import Usd, Sdf, Gf, Vt, Rt


def get_fabric_data_for_prim(stage_id, path):
    """Get the Fabric data for a path as a string"""
    if path is None:
        return "Nothing selected"

    stage = Usd.Stage.Attach(stage_id)

    # If a prim does not already exist in Fabric,
    # it will be fetched from USD by simply creating the
    # Usd.Prim object. At this time, only the attributes that have
    # authored opinions will be fetch into Fabric.
    prim = stage.GetPrimAtPath(Sdf.Path(path))
    if not prim:
        return f"Prim at path {path} is not in Fabric"

    # This diverges a bit from USD - only attributes
    # that exist in Fabric are returned by this API
    attrs = prim.GetAttributes()

    result = f"Fabric data for prim at path {path}\n\n\n"
    for attr in attrs:
        try:
            data = attr.Get()
            datastr = str(data)
            if data is None:
                datastr = "<no value>"
            elif is_vtarray(data):
                datastr = condensed_vtarray_str(data)

        except TypeError:
            # Some data types not yet supported in Python
            datastr = "<no Python conversion>"

        result += "{} ({}): {}\n".format(attr.GetName(), str(attr.GetTypeName().GetAsToken()), datastr)

    return result


def is_vtarray(obj):
    """Check if this is a VtArray type

    In Python, each data type gets its own
    VtArray class i.e. Vt.Float3Array etc.
    so this helper identifies any of them.
    """
    return hasattr(obj, "IsFabricData")


def condensed_vtarray_str(data):
    """Return a string representing VtArray data

    Include at most 6 values, and the total items
    in the array
    """
    size = len(data)
    if size > 6:
        datastr = "[{}, {}, {}, .. {}, {}, {}] (size: {})".format(
            data[0], data[1], data[2], data[-3], data[-2], data[-1], size
        )
    elif size > 0:
        datastr = "["
        for i in range(size - 1):
            datastr += str(data[i]) + ", "
        datastr += str(data[-1]) + "]"
    else:
        datastr = "No data"

    return datastr
