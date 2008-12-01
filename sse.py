#!/usr/bin/env python
#
# This is built on VTK examples, there may still be strange comments
#

#
# First we include the VTK Python packages that will make available
# all of the VTK commands to Python.
#
import vtk, sys, os

# PDB Helix extracting stuff
global atom_defs_by_id
global sheet_defs
global helix_defs
global res_atoms_by_resid
global sorted_residues

atom_defs_by_id = {}
sheet_defs = []
helix_defs = []
res_atoms_by_resid = {}
# sorted_residues = (resid, (x,y,z))
sorted_residues = []

# The first are flags, the tuple is of file extension and handler
g_type_handlers = {"vol": (".vtk", "show_density"),
                   "laplace": (".laplace.vtk", "show_laplace"),
                   "pdb": (".pdb", "show_helices")}


g_op_map = {"vol":     "show_density",
            "lap":     "show_laplace",
            "lpl":     "show_laplace",
            "laplace": "show_laplace",

            "hel":   "show_alpha_helices_wrapper",
            "helix": "show_alpha_helices_wrapper",
            "alpha": "show_alpha_helices_wrapper",

            "bet":   "show_beta_sheets_wrapper",
            "sheet": "show_beta_sheets_wrapper",
            "beta":  "show_beta_sheets_wrapper"}


# 1-based, to make easier to match to spec
def text_at(line, start, end):
    return line[start - 1:end - start + 1 - 1]

# 1-based, to make easier to match to spec
def tok_at(line, start, end):
#    print "line: %s, (%s)%d - (%s)%d" % (line, type(start), start, type(end), end)
    return line[start - 1: end - 1 + 1].strip()

# 1-based, to make easier to match to spec
def float_at(line, start, end):
#    print "line:\n%s, %d - %d" % (line, start, end)
    return float(tok_at(line, start, end))

# 1-based, to make easier to match to spec
def int_at(line, start, end):
    return int(tok_at(line, start, end))

def residue_index(res0, res1):
    global sorted_residues

    (first, last) = (-1, -1)

    print "res0, res1: ", res0, res1
#    print "r[0]: ", sorted_residues[0][0]
    # find index of res0, res1
    first = map(lambda r: r[0], sorted_residues).index(res0)
    last = map(lambda r: r[0], sorted_residues).index(res1)
#    print "fst, lst: ", first, last
#    for x in range(first, last + 1):
#        print "res[", x, "]", sorted_residues[x][0]
    return (first, last)


# def calculate_res_pos(atoms):
#     (x, y, z) = (0.0, 0.0, 0.0)
#     for a in atoms:
#         x += a.x
#         y += a.y
#         z += a.z
#     x /= atoms.length()
#     y /= atoms.length()
#     z /= atoms.length()
#     return (x,y,z)

def make_resid(chainno, resno):
#    print "chainno: ", chainno, "resno:", resno
    return "%3.3s:%5.5s" % (chainno, resno)

def load_helix_def(line):
    chain = tok_at(line, 20, 20)
    start_res_no = tok_at(line, 23, 26)
    start_res_id = make_resid(chain, start_res_no)
    end_res_no = tok_at(line, 34, 37)
    end_res_id = make_resid(chain, end_res_no)
    return (start_res_id, end_res_id)

# 42+ is for registration
#SHEET    2   A 5 ILE 1 184  ARG 1 190 -1  N  LEU 1 186   O  ILE 1 223
def load_sheet_def(line):
    # 12-14 sheet id
    chain = tok_at(line, 22, 22)
    start_res_no = tok_at(line, 23, 26)
    start_res_id = make_resid(chain, start_res_no)
    end_res_no = tok_at(line, 34, 37)
    end_res_id = make_resid(chain, end_res_no)
    return (start_res_id, end_res_id)

def load_atom_def(line):
    atomid = tok_at(line, 7, 11)

    # chain no, residue within that chain
    chain = tok_at(line, 22, 22)
    resno = tok_at(line, 23, 26)

    # ... not sure this id is unique!
    resid = make_resid(chain, resno)
    x = float_at(line, 31, 38)
    y = float_at(line, 39, 46)
    z = float_at(line, 47, 54)
    return (atomid, resid, (x, y, z))


def load_pdb(file):
    for line in file:
        if line.startswith("HELIX"):
            tup = load_helix_def(line)
            helix_defs.append(tup)
        elif line.startswith("SHEET"):
            tup = load_sheet_def(line)
            sheet_defs.append(tup)
        elif line.startswith("ATOM"):
            tup = load_atom_def(line)
            # atom id, straight 1 - N index (in tup[0])
            atom_defs_by_id[tup[0]] = tup
            # tup[1] is a non-unique residue id
            res_atoms_by_resid.setdefault(tup[1], []).append(tup)

    process_residues()

# looks like it just finds center of mass
def process_residues():
    (xmin,ymin,zmin,xmax,ymax,zmax) = (1e38,1e38,1e38,-1e38,-1e38,-1e38)
    global sorted_residues
    for resid, atoms in res_atoms_by_resid.items():
        num_atoms = len(atoms)
        # x,y,z = center of mass it looks like
        (x, y, z) = (0.0, 0.0, 0.0)
        for a in atoms:
            x += a[2][0]; y += a[2][1]; z += a[2][2]
        x /= num_atoms; y /= num_atoms; z /= num_atoms
        xmax = max(x, xmax) ; ymax = max(y, ymax) ; zmax = max(z, zmax)
        xmin = min(x, xmin) ; ymin = min(y, ymin) ; zmin = min(z, zmin)
        sorted_residues.append((resid, (x, y, z)))
    # sort by id ([0])
    sorted_residues.sort(key=lambda r: r[0])
    print "min: %f, %f, %f; max: %f, %f, %f" % (xmin, ymin, zmin, xmax, ymax, zmax)


# by residue center, nothing else
def make_polyline(def_tup):
    global sorted_residues
    (start_res, end_res) = def_tup
#    print "start res", start_res, "end res", end_res
#    print "Looking for helix, from >%s< -> >%s<" % (start_res, end_res)
#    print "\n".join(map(lambda r: ">"+r[0]+"<", sorted_residues))
    #"len sorted reses: ", len(sorted_residues)
    (start_idx, end_idx) = residue_index(start_res, end_res)
    assert -1 < start_idx and -1 < end_idx, "strt: %s, end %s" % (start_res, end_res)
    # +1 as we want /inclusive/
#    print "start idx", start_idx, "end", end_idx
    residues = sorted_residues[start_idx : end_idx + 1]

    polyLinePoints = vtk.vtkPoints()
    polyLinePoints.SetNumberOfPoints(len(residues))
    aPolyLine = vtk.vtkPolyLine()
    aPolyLine.GetPointIds().SetNumberOfIds(len(residues))
    for i,r in enumerate(residues):
        (x, y, z) = r[1]
#        print "pt: ", x, y, z
        polyLinePoints.InsertPoint(i, x, y, z)
        aPolyLine.GetPointIds().SetId(i, i)
    return (polyLinePoints, aPolyLine)


def show_sheets(renderer):
#    print len(sheet_defs), " sheets"
    for sheet in sheet_defs:
#    sheet = sheet_defs[1]
        aPolyLineGrid = vtk.vtkUnstructuredGrid()
        aPolyLineGrid.Allocate(5, 1)

        (polyLinePoints, aPolyLine) = make_polyline(sheet)

#         # Create a tube filter to represent the lines as tubes.  Set up the
#         # associated mapper and actor.
#         tuber = vtk.vtkTubeFilter()
#         tuber.SetInputConnection(appendF.GetOutputPort())
#         tuber.SetRadius(0.1)
#         lineMapper = vtk.vtkPolyDataMapper()
#         lineMapper.SetInputConnection(tuber.GetOutputPort())
#         lineActor = vtk.vtkActor()
#         lineActor.SetMapper(lineMapper)

        aPolyLineGrid.InsertNextCell(aPolyLine.GetCellType(),
                                     aPolyLine.GetPointIds())
        aPolyLineGrid.SetPoints(polyLinePoints)

        aPolyLineMapper = vtk.vtkDataSetMapper()
        aPolyLineMapper.SetInput(aPolyLineGrid)
        aPolyLineActor = vtk.vtkActor()
        aPolyLineActor.SetMapper(aPolyLineMapper)
        aPolyLineActor.GetProperty().SetDiffuseColor(0, 0, 0)

        renderer.AddActor(aPolyLineActor)


def show_alpha_helices(renderer):
    for helix in helix_defs:
#    helix = helix_defs[1]
        aPolyLineGrid = vtk.vtkUnstructuredGrid()
        aPolyLineGrid.Allocate(5, 1)

        (polyLinePoints, aPolyLine) = make_polyline(helix)

#         # Create a tube filter to represent the lines as tubes.  Set up the
#         # associated mapper and actor.
#         tuber = vtk.vtkTubeFilter()
#         tuber.SetInputConnection(appendF.GetOutputPort())
#         tuber.SetRadius(0.1)
#         lineMapper = vtk.vtkPolyDataMapper()
#         lineMapper.SetInputConnection(tuber.GetOutputPort())
#         lineActor = vtk.vtkActor()
#         lineActor.SetMapper(lineMapper)

        aPolyLineGrid.InsertNextCell(aPolyLine.GetCellType(),
                                     aPolyLine.GetPointIds())
        aPolyLineGrid.SetPoints(polyLinePoints)

        aPolyLineMapper = vtk.vtkDataSetMapper()
        aPolyLineMapper.SetInput(aPolyLineGrid)
        aPolyLineActor = vtk.vtkActor()
        aPolyLineActor.SetMapper(aPolyLineMapper)
        aPolyLineActor.GetProperty().SetDiffuseColor(1, 1, 1)

        renderer.AddActor(aPolyLineActor)


def show_beta_sheets(renderer):
    for sheet in sheet_defs:
#    helix = helix_defs[1]
        aPolyLineGrid = vtk.vtkUnstructuredGrid()
        aPolyLineGrid.Allocate(5, 1)

        (polyLinePoints, aPolyLine) = make_polyline(sheet)

#         # Create a tube filter to represent the lines as tubes.  Set up the
#         # associated mapper and actor.
#         tuber = vtk.vtkTubeFilter()
#         tuber.SetInputConnection(appendF.GetOutputPort())
#         tuber.SetRadius(0.1)
#         lineMapper = vtk.vtkPolyDataMapper()
#         lineMapper.SetInputConnection(tuber.GetOutputPort())
#         lineActor = vtk.vtkActor()
#         lineActor.SetMapper(lineMapper)

        aPolyLineGrid.InsertNextCell(aPolyLine.GetCellType(),
                                     aPolyLine.GetPointIds())
        aPolyLineGrid.SetPoints(polyLinePoints)

        aPolyLineMapper = vtk.vtkDataSetMapper()
        aPolyLineMapper.SetInput(aPolyLineGrid)
        aPolyLineActor = vtk.vtkActor()
        aPolyLineActor.SetMapper(aPolyLineMapper)
        aPolyLineActor.GetProperty().SetDiffuseColor(1, 1, 1)

        renderer.AddActor(aPolyLineActor)


def show_density(fname, ren):
    # originally 60.0
    FEATURE_ANGLE = 60.0

# 1AGW goes from 0 - 0.6+
    MAX_DENSITY = 1.0
    MIN_DENSITY = 0.0
#    MAX_DENSITY = 1.0
    STEPS = 20
    OPACITY = 1.0 / STEPS

    # The following reader is used to read a series of 2D slices (images)
    # that compose the volume. The slice dimensions are set, and the
    # pixel spacing. The data Endianness must also be specified. The reader
    # usese the FilePrefix in combination with the slice number to construct
    # filenames using the format FilePrefix.%d. (In this case the FilePrefix
    # is the root name of the file: quarter.)
    mrc = vtk.vtkStructuredPointsReader()
    #v16.SetDataDimensions(64, 64)
    #v16.SetDataByteOrderToLittleEndian()
    #v16.SetFilePrefix(VTK_DATA_ROOT + "/Data/headsq/quarter")
    #v16.SetImageRange(1, 93)
    #v16.SetDataSpacing(3.2, 3.2, 1.5)
    mrc.SetFileName(fname)
    #mrc.SetFileName(VTK_DATA_ROOT + "/Data/ironProt.vtk")

    skinExtractor = vtk.vtkContourFilter()
    skinExtractor.UseScalarTreeOn()
    # Not sure whether this is used by the alg..
#    skinExtractor.ComputeGradientsOn()
    skinExtractor.SetInputConnection(mrc.GetOutputPort())
    density_range = MAX_DENSITY - MIN_DENSITY
    for i in range(STEPS):
        density = float(i) / float(STEPS) * density_range
    #    print "density:", density
        skinExtractor.SetValue(i, density + MIN_DENSITY)

    skinNormals = vtk.vtkPolyDataNormals()
    skinNormals.SetInputConnection(skinExtractor.GetOutputPort())
    skinNormals.SetFeatureAngle(FEATURE_ANGLE)
    # optional extras
#    skinNormals.SplittingOn()
#    skinNormals.ComputePointNormalsOn()

    skinStripper = vtk.vtkStripper()
    skinStripper.SetInputConnection(skinNormals.GetOutputPort())

    skinMapper = vtk.vtkPolyDataMapper()
    skinMapper.SetInputConnection(skinStripper.GetOutputPort())
    skinMapper.ScalarVisibilityOff()

    skin = vtk.vtkActor()
    skin.SetMapper(skinMapper)
    # Orangish, I guess?
    skin.GetProperty().SetDiffuseColor(1, .49, 0.25)
    skin.GetProperty().SetSpecular(.3)
    skin.GetProperty().SetSpecularPower(20)

    # An outline provides context around the data.
    outlineData = vtk.vtkOutlineFilter()
    outlineData.SetInputConnection(mrc.GetOutputPort())
    mapOutline = vtk.vtkPolyDataMapper()
    mapOutline.SetInputConnection(outlineData.GetOutputPort())
    outline = vtk.vtkActor()
    outline.SetMapper(mapOutline)
    outline.GetProperty().SetColor(0, 0, 0)

#     # It is convenient to create an initial view of the data. The FocalPoint
#     # and Position form a vector direction. Later on (ResetCamera() method)
#     # this vector is used to position the camera to look at the data in
#     # this direction.
#     aCamera = vtk.vtkCamera()
#     aCamera.SetViewUp(0, 0, -1)
#     aCamera.SetPosition(0, 1, 0)
#     aCamera.SetFocalPoint(0, 0, 0)
#     aCamera.ComputeViewPlaneNormal()

    # Actors are added to the renderer.
    ren.AddActor(outline)
    ren.AddActor(skin)

    # Set skin to semi-transparent.
    #skin.VisibilityOff()
    skin.GetProperty().SetOpacity(OPACITY)


def show_laplace(fname, ren):
    # originally 60.0
    FEATURE_ANGLE = 30.0

#    MAX_DENSITY = 0.6
    OPACITY = 0.1
    (RED, GREEN, BLUE) = (0.0, 0.0, 0.9)

    mrc = vtk.vtkStructuredPointsReader()
    mrc.SetFileName(fname)

#GetOrigin()
#GetSpacing()
#GetExtent()
#vtk.vtkStructuredPoints = mrc.GetOutput()

    # An isosurface, or contour value of 500 is known to correspond to the
    # skin of the patient. Once generated, a vtkPolyDataNormals filter is
    # is used to create normals for smooth surface shading during rendering.
    # The triangle stripper is used to create triangle strips from the
    # isosurface these render much faster on many systems.
    skinExtractor = vtk.vtkContourFilter()
    skinExtractor.UseScalarTreeOn()
    # Not sure whether this is used by the alg..
#    skinExtractor.ComputeGradientsOn()
    skinExtractor.SetInputConnection(mrc.GetOutputPort())
    #    print "density:", density
    skinExtractor.SetValue(0, 0.0)

    skinNormals = vtk.vtkPolyDataNormals()
    skinNormals.SetInputConnection(skinExtractor.GetOutputPort())
    skinNormals.SetFeatureAngle(FEATURE_ANGLE)
    # optional extras
#    skinNormals.SplittingOn()
#    skinNormals.ComputePointNormalsOn()

    skinStripper = vtk.vtkStripper()
    skinStripper.SetInputConnection(skinNormals.GetOutputPort())

    skinMapper = vtk.vtkPolyDataMapper()
    skinMapper.SetInputConnection(skinStripper.GetOutputPort())
    skinMapper.ScalarVisibilityOff()

    skin = vtk.vtkActor()
    skin.SetMapper(skinMapper)
    skin.GetProperty().SetDiffuseColor(RED, GREEN, BLUE)
    skin.GetProperty().SetSpecular(.3)
    skin.GetProperty().SetSpecularPower(20)

    # An outline provides context around the data.
    outlineData = vtk.vtkOutlineFilter()
    outlineData.SetInputConnection(mrc.GetOutputPort())
    mapOutline = vtk.vtkPolyDataMapper()
    mapOutline.SetInputConnection(outlineData.GetOutputPort())
    outline = vtk.vtkActor()
    outline.SetMapper(mapOutline)
    outline.GetProperty().SetColor(0, 0, 0)

#     # It is convenient to create an initial view of the data. The FocalPoint
#     # and Position form a vector direction. Later on (ResetCamera() method)
#     # this vector is used to position the camera to look at the data in
#     # this direction.
#     aCamera = vtk.vtkCamera()
#     aCamera.SetViewUp(0, 0, -1)
#     aCamera.SetPosition(0, 1, 0)
#     aCamera.SetFocalPoint(0, 0, 0)
#     aCamera.ComputeViewPlaneNormal()

    # Actors are added to the renderer.
    ren.AddActor(outline)
    ren.AddActor(skin)

    # Set skin to semi-transparent.
    #skin.VisibilityOff()
    skin.GetProperty().SetOpacity(OPACITY)


def show_alpha_helices_wrapper(fname, ren):
    f = open(fname)
    load_pdb(f)

    show_alpha_helices(ren)


def show_beta_sheets_wrapper(fname, ren):
    f = open(fname)
    load_pdb(f)

    show_beta_sheets(ren)


def show_axes(ren):
    # Create the axes and the associated mapper and actor.
    axes = vtk.vtkAxes()
    axes.SetOrigin(0, 0, 0)
    # length - hardwired for our images, which are usually 80 on a side
    # (80/2 = 40)
    AxisLength = 35.0
    TextScale = 1.5
    TextR = 0.4
    TextG = 0.4
    TextB = 0.8

    axes.SetScaleFactor(AxisLength)

    axesMapper = vtk.vtkPolyDataMapper()
    axesMapper.SetInputConnection(axes.GetOutputPort())
    axesActor = vtk.vtkActor()
    axesActor.SetMapper(axesMapper)

    # Label the axes
    XText = vtk.vtkVectorText()
    XText.SetText("X")

    XTextMapper = vtk.vtkPolyDataMapper()
    XTextMapper.SetInputConnection(XText.GetOutputPort())

    XActor = vtk.vtkFollower()
    XActor.SetMapper(XTextMapper)
    XActor.SetScale(TextScale, TextScale, TextScale)
    XActor.SetPosition(AxisLength, 0.0, 0.0)
    XActor.GetProperty().SetColor(TextR, TextG, TextB)

    YText = vtk.vtkVectorText()
    YText.SetText("Y")

    YTextMapper = vtk.vtkPolyDataMapper()
    YTextMapper.SetInputConnection(YText.GetOutputPort())

    YActor = vtk.vtkFollower()
    YActor.SetMapper(YTextMapper)
    YActor.SetScale(TextScale, TextScale, TextScale)
    YActor.SetPosition(0.0, AxisLength, 0.0)
    YActor.GetProperty().SetColor(TextR, TextG, TextB)

    ZText = vtk.vtkVectorText()
    ZText.SetText("Z")

    ZTextMapper = vtk.vtkPolyDataMapper()
    ZTextMapper.SetInputConnection(ZText.GetOutputPort())

    ZActor = vtk.vtkFollower()
    ZActor.SetMapper(ZTextMapper)
    ZActor.SetScale(TextScale, TextScale, TextScale)
    ZActor.SetPosition(0.0, 0.0, AxisLength)
    ZActor.GetProperty().SetColor(TextR, TextG, TextB)


    ren.AddActor(axesActor)
    ren.AddActor(XActor)
    ren.AddActor(YActor)
    ren.AddActor(ZActor)


def make_pairs(lst):
    if not lst:
        return []
    else:
        return [(lst[0], lst[1])] + make_pairs(lst[2:])


# eg, 1AGW, [vol, laplace]
def main(pairs):

    import __main__ as _main_module

    renderer = vtk.vtkRenderer()

    for op, file in pairs:
        handler_name = g_op_map[op]
        handler = _main_module.__dict__[handler_name]
        fname = file
        if not os.access(fname, os.R_OK):
            print "Missing %s" % (fname)
        else:
            handler(fname, renderer)

    show_axes(renderer)

    # Create the usual rendering stuff.
    renWin = vtk.vtkRenderWindow()
    renWin.AddRenderer(renderer)
    renWin.SetSize(1000, 750)
    iren = vtk.vtkRenderWindowInteractor()
    style = vtk.vtkInteractorStyleTrackballCamera()
    iren.SetInteractorStyle(style)
    iren.SetRenderWindow(renWin)

    # color
    renderer.SetBackground(.1, .2, .4)

    # Render the scene and start interaction.
    iren.Initialize()
    renWin.Render()
    iren.Start()

# -- OLD # To show a volume, ./sse.py /big/common/htdata/1AGW vol (and it'll look for /.../1AGW.vtk. Yah.)
# pairs: op file
# ./sse.py vol /big/common/htdata/seeded.vtk lap /big/common/htdata/1AGW.laplace.vtk hel /big/common/htdata/1AGW.pdb
if __name__ == "__main__":
#    basename = sys.argv[1]
#    parms = sys.argv[2:]
    args = sys.argv[1:]
    if (len(args) % 2) != 0:
        print "Need pairs, of op (eg vol, or beta) then file"
    else:
        pairs = make_pairs(args)
        print pairs
        # eg laplace, gradient, whatever else
        main(pairs)
