bl_info = {
    "name": "Export M2N",
    "category": "Export",
}

import os, bpy, bmesh
from bpy.props import *
from struct import *
from bpy_extras.io_utils import ExportHelper

assetsMaterialDir = 'assets/materials/'
assetsTextureDir = 'assets/textures/'
assetsSceneDir = 'assets/scenes/'
assetsModelDir = 'assets/models/'
assetsGeometryDir = 'assets/geometry/'

materialNameFormat = "'material:" + assetsMaterialDir + "%s.mat'"
modelNameFormat = "'model:" + assetsModelDir + "%s.model'"
geometryNameFormat = "'geom:" + assetsGeometryDir + "%s.geom'"

class ExportM2N(bpy.types.Operator, ExportHelper):
    """Export M2N scene"""
    bl_idname = "exportm2n.folder"
    bl_label = "Export M2N scene"

    filename_ext = "."
    use_filter_folder = True

    models = {}

    ns = StringProperty(
        name = 'Namespace',
        default = ''
    )
    overrideScenes = BoolProperty(
        name = 'Override existing scenes',
        default = True
    )
    overrideModels = BoolProperty(
        name = 'Override existing models',
        default = False
    )
    overrideImages = BoolProperty(
        name = 'Override existing images',
        default = False
    )
    overrideGeometry = BoolProperty(
        name = 'Override existing geometry',
        default = False
    )
    overrideMaterials = BoolProperty(
        name = 'Override existing materials',
        default = False
    )
    entireScene = BoolProperty(
        name = 'Export entire scene',
        default = False
    )

    def meshToStr(self, obj):
        loc = obj.location
        rot = obj.matrix_local.to_euler('XYZ')
        scl = obj.scale

        tx =  loc.x
        ty =  loc.z
        tz = -loc.y
        rx =  rot.x
        ry =  rot.z
        rz = -rot.y
        sx =  scl.x
        sy =  scl.z
        sz =  scl.y

        if tx == 0 and ty == 0 and tz == 0 and rx == 0 and ry == 0 and rz == 0 and sx == 1 and sy == 1 and sz == 1 and len(obj.children) == 0:
            return modelNameFormat % (self.ns + obj.data.name)
        else:
            strings = [modelNameFormat % (self.ns + obj.data.name)]
            if tx != 0: strings.append('tx = ' + str(tx))
            if ty != 0: strings.append('ty = ' + str(ty))
            if tz != 0: strings.append('tz = ' + str(tz))
            if rx != 0: strings.append('rx = ' + str(rx))
            if ry != 0: strings.append('ry = ' + str(ry))
            if rz != 0: strings.append('rz = ' + str(rz))
            if sx != 1: strings.append('sx = ' + str(sx))
            if sy != 1: strings.append('sy = ' + str(sy))
            if sz != 1: strings.append('sz = ' + str(sz))

            for child in obj.children:
                strings.append("['" + child.name + "'] = " + self.meshToStr(child).replace('\n', '\n\t'))

            return '{\n\t' + ',\n\t'.join(strings) + '\n}'

    def modelToStr(self, name, model):
        matStrings = []
        for material in model:
            geomName = name + '_' + material
            matStrings.append(
                "['" + material + "'] = {\n\t'mesh',\n\t" +
                "geometry = " + (geometryNameFormat % (self.ns + geomName)) + ",\n\t"
                "material = " + (materialNameFormat % (self.ns + material)) + "\n}"
            )
        return '{\n\t' + ',\n'.join(matStrings).replace('\n', '\n\t') + '\n}'

    def pngFilename(self, filename):
        extPos = filename.rfind('.')
        if extPos >= 0:
            return filename[:filename.rfind('.')] + '.png'
        else:
            return filename + '.png'

    def writeImages(self, path):
        scene = bpy.data.scenes.new('__M2N_EXPORTER_SCENE')
        scene.render.image_settings.color_mode = 'RGBA'
        scene.render.image_settings.file_format = 'PNG'

        directory = path + '/' + assetsTextureDir + self.ns
        if not os.path.exists(directory):
            os.makedirs(directory)

        for image in bpy.data.images.values():
            filepath = directory + '/' + self.pngFilename(image.name)
            
            if not os.path.exists(filepath) and image.source == 'FILE' or self.overrideImages:
                try:
                    image.save_render(filepath, scene)
                except RuntimeError:
                    pass

        bpy.data.scenes.remove(scene)

    def writeMaterials(self, path):
        directory = path + '/' + assetsMaterialDir + self.ns
        if not os.path.exists(directory):
            os.makedirs(directory)

        for material in bpy.data.materials.values():
            filepath = directory + '/' + material.name + '.mat'
            if not os.path.exists(filepath) or self.overrideMaterials:
                out = open(filepath, "w")
                out.write('return {\n')
                textureStrings = []
                for slot, texSlot in enumerate(material.texture_slots):
                    if texSlot is not None and texSlot.texture.type == 'IMAGE':
                        textureStrings.append(
                            "uTexture%i = 'tex2d:%s%s'" %
                            (slot, assetsTextureDir, self.ns + self.pngFilename(texSlot.texture.image.name))
                        )
                if len(textureStrings) > 0:
                    out.write('\ttextures = {\n\t\t%s\n\t}\n' % ',\n\t\t'.join(textureStrings))
                out.write('}')
                out.close()

    def writeScene(self, path):
        directory = path + '/' + assetsSceneDir + self.ns
        if not os.path.exists(directory):
            os.makedirs(directory)

        scene = bpy.context.scene
        filepath = directory + '/' + scene.name + '.scene'
        if not os.path.exists(filepath) or self.overrideScenes:
            out = open(filepath, "w")
            out.write('return {\n\t')

            objStrings = []
            for obj in scene.objects:
                if obj.type == 'MESH' and obj.parent is None:
                    objStrings.append("['" + obj.name + "'] = " + self.meshToStr(obj))

            out.write(',\n'.join(objStrings).replace('\n', '\n\t') + '\n}')
            out.close()

    def writeModels(self, path):
        directory = path + '/' + assetsModelDir + self.ns
        if not os.path.exists(directory):
            os.makedirs(directory)

        for name in self.models:
            model = self.models[name]
            filepath = directory + '/' + name + '.model'
                
            if not os.path.exists(filepath) or self.overrideModels:
                out = open(filepath, "w")
                out.write('return ' + self.modelToStr(name, model))
                out.close()

    def writeGeometry(self, path):
        directory = path + '/' + assetsGeometryDir + self.ns
        if not os.path.exists(directory):
            os.makedirs(directory)

        for name in self.models:
            model = self.models[name]
            for material in model:
                geomName = name + '_' + material
                filepath = directory + '/' + geomName + '.geom'
                vertList = model[material]

                out = open(filepath, "wb")
                out.write(pack('<II', len(vertList) * 20, 0))

                for vert in vertList:
                    pos = vert['position']
                    tc0 = vert['texCoords0']

                    out.write(pack('<fffff', pos[0], pos[1], pos[2], tc0[0], tc0[1]))

                out.close()

    def write(self, filename):
        if len(self.ns) > 0:
            self.ns += '/'

        path = os.path.dirname(os.path.realpath(filename))
        if len(bpy.context.scene.objects) > 1 or self.entireScene:
            self.writeScene(path)

        self.writeMaterials(path)
        self.writeImages(path)
        self.writeModels(path)
        self.writeGeometry(path)

    def execute(self, context):        # execute() is called by blender when running the operator.
        for obj in bpy.context.scene.objects:
            if obj.type == 'MESH':
                m = {}

                bm = bmesh.new()
                bm.from_mesh(obj.data)
                bmesh.ops.triangulate(bm, faces=bm.faces)

                uv_layer = bm.loops.layers.uv.active
                for face in bm.faces:
                    mat = bpy.data.materials.values()[face.material_index]
                    if mat is not None:
                        mat = mat.name

                    if mat not in m:
                        m[mat] = []

                    for loop, vert in zip(face.loops, face.verts):
                        vert = {
                            'position': (vert.co.x, vert.co.z, -vert.co.y),
                            'normal': (vert.normal.x, vert.normal.z, -vert.normal.y)
                        }

                        if uv_layer is not None:
                            vert['texCoords0'] = (loop[uv_layer].uv.x, 1 - loop[uv_layer].uv.y)
                        else:
                            vert['texCoords0'] = (0.0, 0.0)

                        m[mat].append(vert)

                bm.free()
                del bm

                self.models[obj.data.name] = m

        userpath = self.properties.filepath
        self.write(userpath)

        return {'FINISHED'}            # this lets blender know the operator finished successfully.

def menu_func_export(self, context):
    self.layout.operator(ExportM2N.bl_idname, text="Export M2N Scene")

def register():
    bpy.utils.register_class(ExportM2N)
    bpy.types.INFO_MT_file_export.append(menu_func_export)

def unregister():
    bpy.utils.unregister_class(ExportM2N)
    bpy.types.INFO_MT_file_export.remove(menu_func_export)

# This allows you to run the script directly from blenders text editor
# to test the addon without having to install it.
if __name__ == "__main__":
    register()
