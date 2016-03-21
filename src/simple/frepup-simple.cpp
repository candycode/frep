#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osg/Camera>
#include <osgGA/TrackballManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/CompositeViewer>
#include <osg/io_utils>
#include <osg/MatrixTransform>
#include <osg/ref_ptr>
#include <osg/TextureCubeMap>
#include <osg/TextureRectangle>

#include <cassert>
#include <cctype>

#include <exception>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include "frepup-shaders.h"
#include "aspect_ratio.h"


#include <osg/ComputeBoundsVisitor>

//------------------------------------------------------------------------------
std::string ReadFile(const char *fname) {
    assert(fname);
    std::ifstream is(fname);
    if (!is.good())
        throw std::logic_error("Cannot open file "
                               + std::string(fname));
    std::string buffer;
    std::string output;
    while (is.good()) {
        std::getline(is, buffer);
        output += buffer;
        output += '\n';
        buffer.clear();
    }
    return output;
}


//------------------------------------------------------------------------------
Primitives ReadPrimitives(const char *fname) {
    assert(fname);
    std::ifstream is(fname);
    std::string buffer;
    Primitives primitives;
    static const std::string FREP_PLACEHOLDER("$F");
    static const std::string GRADIENT_PLACEHOLDER("$G");
    static const std::string COLOR_PLACEHOLDER("ComputeColor");
    static const std::string INTERSECT_PLACEHOLDER("$I");
    while (is.good()) {
        std::getline(is, buffer);
        if (buffer.empty()) continue;
        std::istringstream iss(buffer);
        if (!iss.good())
            throw std::logic_error(
                    "Error reading primitive file");
        std::string objname;
        std::string filename;
        std::string matFilename;
        iss >> objname;
        if (!iss.good())
            throw std::logic_error(
                    "Error reading primitive file name");
        iss >> matFilename;
        if (!iss.good())
            throw std::logic_error(
                    "Error reading primitive maetrial file name");
        iss >> filename;
        buffer.clear();
        std::string frepCode = ReadFile(filename.c_str());
        if (frepCode.empty())
            throw std::logic_error(
                    "Error reading from file " + filename);
        std::string matCode = ReadFile(matFilename.c_str());
        if (matCode.empty())
            throw std::logic_error(
                    "Error reading from file " + matFilename);
        std::string frepname = objname + "_F";
        std::string gradname = objname + "_G";
        std::string invMatrixName = objname + "_Mi";
        std::string matrixName = objname + "_M";
        std::string invNormalMatrixName = objname + "_Ni";
        std::string normalMatrixName = objname + "_N";
        std::string colorname = objname + "_Color";
        std::string intersectname = objname + "_I";
        while (frepCode.find(FREP_PLACEHOLDER) != std::string::npos) {
            frepCode.replace(frepCode.find(FREP_PLACEHOLDER),
                             FREP_PLACEHOLDER.size(), frepname);
        }
        while (frepCode.find(GRADIENT_PLACEHOLDER) != std::string::npos) {
            frepCode.replace(frepCode.find(GRADIENT_PLACEHOLDER),
                             GRADIENT_PLACEHOLDER.size(), gradname);
        }
        while (frepCode.find(INTERSECT_PLACEHOLDER) != std::string::npos) {
            frepCode.replace(frepCode.find(INTERSECT_PLACEHOLDER),
                             INTERSECT_PLACEHOLDER.size(), intersectname);
        }
        if (matCode.find(COLOR_PLACEHOLDER) != std::string::npos) {
            matCode.replace(matCode.find(COLOR_PLACEHOLDER),
                            COLOR_PLACEHOLDER.size(), colorname);
        }
        primitives.push_back(PrimitiveEntry(objname, frepname, gradname,
                                            intersectname, colorname,
                                            invMatrixName,
                                            matrixName, invNormalMatrixName,
                                            normalMatrixName,
                                            frepCode, matCode));
    }

    return primitives;
}

//------------------------------------------------------------------------------
std::string ReplaceNamesInComposite(const char *fname,
                                    const Primitives &primitives) {
    std::string compositeCode = ReadFile(fname);
    if (compositeCode.empty())
        throw std::logic_error("Cannot read composite code");
    std::string::size_type s = compositeCode.find("$");
    while (s != std::string::npos) {
        std::string::size_type i = s + 1;
        for (; ::isdigit(compositeCode[i]); ++i);
        if (s + 1 == i) throw std::logic_error("Wrong object specification");
        std::string idx(compositeCode, s + 1, i - s - 1);
        std::istringstream iss(idx);
        int id = 0;
        iss >> id;
        /// @warning we are using 1-based indexing for primitive references
        /// which needs to be converted to zero based vector index!!!
        --id;
        if (id >= int(primitives.size()))
            throw std::logic_error(
                    "Error object index greater than number of objects");
        const std::string tag = "$" + idx;
        compositeCode.replace(compositeCode.find(tag), tag.size(),
                              primitives[id].objname);
        s = compositeCode.find("$");
    }
    return compositeCode;
}

template<class T>
void L_(const std::string &m, const T &v) {
    std::clog << m << '\t' << v << '\n';
}


//------------------------------------------------------------------------------
int main(int argc, char **argv) {
    try {
        osg::ArgumentParser arguments(&argc, argv);

        //1) Read primitives and composite information
        std::string primitivesFileName;
        std::string compositeFileName;
        std::string vertexShaderFileName;
        std::string fragmentShaderFileName;
        std::string compositeMatFileName;
        std::string compositeTexture;
        std::string compositeTextureUniform("displacement");
        float dPrimitiveStep = 0.06f;
        float dCompositeStep = 0.025f;
        float compositeBoxSize = 1.5f;
        float primitiveBoxSize = 1.15f;
        if (!arguments.read("-primitives", primitivesFileName))
            throw std::logic_error("Missing primitives file name");
        if (!arguments.read("-composite", compositeFileName))
            throw std::logic_error("Missing composite file name");
        if (!arguments.read("-compositeMat", compositeMatFileName))
            throw std::logic_error("Missing composite material file name");
        if (!arguments.read("-vert", vertexShaderFileName))
            throw std::logic_error("Missing vertex shader file name");
        if (!arguments.read("-frag", fragmentShaderFileName))
            throw std::logic_error("Missing fragment shader file name");
        arguments.read("-dPrimitiveStep", dPrimitiveStep);
        arguments.read("-dCompositeStep", dCompositeStep);
        arguments.read("-compositeBoxSize", compositeBoxSize);
        arguments.read("-primitiveBoxSize", primitiveBoxSize);
        arguments.read("-compositeTexture", compositeTexture);
        arguments.read("-compositeTextureUniform", compositeTextureUniform);
        const bool computeRayInFragmentShader = arguments.read(
                "-computeRayInFragmentShader");
        const bool optimizedComposite = arguments.read("-optimizedComposite");
        const bool cubeMap = arguments.read("-cubeMap");
        Primitives primitives = ReadPrimitives(primitivesFileName.c_str());
        L_("READ PRIMITIVES", primitives.size());
        std::string compositeCode = ReplaceNamesInComposite(
                compositeFileName.c_str(), primitives);
        L_("REPLACED NAMES IN COMPOSITE", "ok");
        std::string vertexShaderText = ReadFile(vertexShaderFileName.c_str());
        std::string fragmentShaderText = ReadFile(
                fragmentShaderFileName.c_str());
        L_("READ SHADERS", "ok");
        std::string compositeMaterial = ReplaceNamesInComposite(
                compositeMatFileName.c_str(), primitives);
        L_("REPLACED NAMES IN MATERIAL", "ok");
        //2) Create primitive scenegraph with manipulators
        std::vector<osg::ref_ptr < osg::Node> > primitiveNodes;
        osg::ref_ptr <osg::Group> primitiveRoot = new osg::Group;
        for (Primitives::iterator i = primitives.begin();
             i != primitives.end(); ++i) {
            i->boxSizeX = primitiveBoxSize;
            i->boxSizeY = primitiveBoxSize;
            i->boxSizeZ = primitiveBoxSize;
            //the manipTransoform of the primitive instance is filled
            //with the transform modified by the manipulator
            osg::ref_ptr <osg::Group> pg =
                    CreatePrimitiveSceneGraph(*i,
                                              vertexShaderText,
                                              fragmentShaderText);
            L_("\tPRIMITIVE", "ok");
            primitiveNodes.push_back(pg);
            primitiveRoot->addChild(osg::get_pointer(pg));
        }
        osg::ref_ptr <osg::StateSet> primitiveSSet =
                                        primitiveRoot->getOrCreateStateSet();
        primitiveSSet->addUniform(
                new osg::Uniform("deltaStep", dPrimitiveStep));

        L_("CREATED PRIMITIVE SCENEGRAPH", "ok");

        //3) Create composite scenegraph
        osg::ref_ptr <osg::Box> compositeBox;
        osg::ref_ptr <osg::Uniform> halfBoxSize;
        osg::ref_ptr <osg::Uniform> boxCenter;
        osg::ref_ptr <osg::Group> compositeRoot = new osg::Group;

        BoxVector compositeBoxes;
        UniformVector compositeHalfBoxSizes;
        UniformVector primitiveMatrices;

        const bool depthMap = true;
        osg::ref_ptr <osg::Camera> depthCamera;
        osg::ref_ptr <osg::Uniform> depthCameraViewportUniform;
        if (depthMap) {
            extern osg::TextureRectangle *GenerateDepthTextureRectangle();
            extern osg::Camera *CreatePreRenderCamera(osg::Texture *depth,
                                                      osg::Texture *positions,
                                                      osg::Texture *normals);
            osg::ref_ptr <osg::TextureRectangle> t =
                                            GenerateDepthTextureRectangle();
            depthCamera = CreatePreRenderCamera(osg::get_pointer(t), 0, 0);
            depthCameraViewportUniform = new osg::Uniform("ssaoviewport",
                                                          osg::Vec4());
            osg::ref_ptr <osg::StateSet> ss =
                                            depthCamera->getOrCreateStateSet();
            compositeRoot->getOrCreateStateSet()
                    ->setTextureAttributeAndModes(1, osg::get_pointer(t));
            compositeRoot->getOrCreateStateSet()->addUniform(
                    new osg::Uniform("depthMap", 1));
            compositeRoot->getOrCreateStateSet()->addUniform(
                    osg::get_pointer(depthCameraViewportUniform));
            depthCamera->addChild(osg::get_pointer(compositeRoot));

        }

        if (optimizedComposite) {

            compositeRoot->addChild(
                    CreateOptimalCompositeSceneGraph(primitives,
                                                     compositeCode,
                                                     compositeMaterial,
                                                     vertexShaderText,
                                                     fragmentShaderText,
                                                     compositeBoxSize,
                                                     compositeBox,
                                                     compositeBoxes,
                                                     compositeHalfBoxSizes,
                                                     primitiveMatrices));

        }
        else {
            compositeRoot->addChild(
                    CreateCompositeSceneGraph(primitives,
                                              compositeCode,
                                              compositeMaterial,
                                              vertexShaderText,
                                              fragmentShaderText,
                                              compositeBoxSize,
                                              compositeBox,
                                              halfBoxSize,
                                              boxCenter));
        }
        osg::ref_ptr <osg::StateSet> compositeSSet =
                compositeRoot->getOrCreateStateSet();
        compositeSSet->addUniform(
                new osg::Uniform("deltaStep", dCompositeStep));
        if (!compositeTexture.empty()) {
            if (!cubeMap) {
                osg::ref_ptr <osg::Texture2D> displacementTexture =
                                                            new osg::Texture2D;
                osg::ref_ptr <osg::Image> image = osgDB::readImageFile(
                        compositeTexture);
                if (!image)
                    throw std::logic_error(
                            "Cannot read image " + compositeTexture);
                const int textureUnit = 0;
                displacementTexture->setImage(osg::get_pointer(image));
                // IMPORTANT TO KEEP NEAREST IF NOT ARTIFACTS AT x=0 ARISE!!
                displacementTexture->setFilter(osg::Texture::MIN_FILTER,
                                               osg::Texture::NEAREST);//LINEAR_MIPMAP_LINEAR);
                displacementTexture->setFilter(osg::Texture::MAG_FILTER,
                                               osg::Texture::NEAREST);//LINEAR_MIPMAP_LINEAR);
                //displacementTexture->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
                //displacementTexture->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );
                displacementTexture->setWrap(osg::Texture::WRAP_S,
                                             osg::Texture::REPEAT);
                displacementTexture->setWrap(osg::Texture::WRAP_T,
                                             osg::Texture::REPEAT);
                compositeSSet->
                    setTextureAttributeAndModes(textureUnit,
                                                osg::get_pointer(
                                                        displacementTexture));
                compositeSSet->addUniform(
                        new osg::Uniform(compositeTextureUniform.c_str(),
                                         textureUnit));
                //compositeSSet->setMode( GL_BLEND, osg::StateAttribute::ON );
            }
            else {
                osg::ref_ptr <osg::TextureCubeMap> cubemap =
                                                        new osg::TextureCubeMap;
                const std::string::size_type FILETYPE_SEPARATOR =
                                                    compositeTexture.rfind(".");
                const std::string tname(compositeTexture, 0,
                                        FILETYPE_SEPARATOR);
                const std::string ext(compositeTexture, FILETYPE_SEPARATOR);
                std::clog << "\n****************** " << FILETYPE_SEPARATOR <<
                ' ' << tname << ' ' << ext << " ***********************\n";
                osg::ref_ptr <osg::Image> imagePosX = osgDB::readImageFile(
                        tname + "_R" + ext);
                osg::ref_ptr <osg::Image> imageNegX = osgDB::readImageFile(
                        tname + "_L" + ext);
                osg::ref_ptr <osg::Image> imagePosY = osgDB::readImageFile(
                        tname + "_T" + ext);
                osg::ref_ptr <osg::Image> imageNegY = osgDB::readImageFile(
                        tname + "_B" + ext);
                osg::ref_ptr <osg::Image> imagePosZ = osgDB::readImageFile(
                        tname + "_N" + ext);
                osg::ref_ptr <osg::Image> imageNegZ = osgDB::readImageFile(
                        tname + "_F" + ext);

                if (imagePosX && imageNegX && imagePosY && imageNegY &&
                    imagePosZ && imageNegZ) {
                    cubemap->setImage(osg::TextureCubeMap::POSITIVE_X,
                                      imagePosX);
                    cubemap->setImage(osg::TextureCubeMap::NEGATIVE_X,
                                      imageNegX);
                    cubemap->setImage(osg::TextureCubeMap::POSITIVE_Y,
                                      imagePosY);
                    cubemap->setImage(osg::TextureCubeMap::NEGATIVE_Y,
                                      imageNegY);
                    cubemap->setImage(osg::TextureCubeMap::POSITIVE_Z,
                                      imagePosZ);
                    cubemap->setImage(osg::TextureCubeMap::NEGATIVE_Z,
                                      imageNegZ);
                    cubemap->setWrap(osg::Texture::WRAP_S,
                                     osg::Texture::CLAMP_TO_EDGE);
                    cubemap->setWrap(osg::Texture::WRAP_T,
                                     osg::Texture::CLAMP_TO_EDGE);
                    cubemap->setWrap(osg::Texture::WRAP_R,
                                     osg::Texture::CLAMP_TO_EDGE);
                    cubemap->setFilter(osg::Texture::MIN_FILTER,
                                       osg::Texture::LINEAR_MIPMAP_NEAREST);
                    cubemap->setFilter(osg::Texture::MAG_FILTER,
                                       osg::Texture::LINEAR);
                }
                else
                    throw std::logic_error(
                            "Cannot read cubemap " + compositeTexture);
                const int textureUnit = 0;
                compositeSSet->setTextureAttributeAndModes(textureUnit,
                                                           osg::get_pointer(
                                                                   cubemap));
                compositeSSet->addUniform(
                        new osg::Uniform(compositeTextureUniform.c_str(),
                                         textureUnit));
            }
        }

        L_("CREATED COMPOSITE SCENEGRAPH", "ok");

        //4) Create composite viewer, left view for primitives, right
        //   view for composite

        osgViewer::CompositeViewer viewer(arguments);

        osg::GraphicsContext::WindowingSystemInterface *wsi =
                            osg::GraphicsContext::getWindowingSystemInterface();
        if (!wsi) {
            osg::notify(osg::NOTICE) <<
            "Error, no WindowSystemInterface available, cannot create windows." <<
            std::endl;
            return 1;
        }

        unsigned int width = 0, height = 0;
        wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0),
                                 width, height);

        const int DEF_WIN_X = 60;
        const int DEF_WIN_Y = 60;
        const int DEF_WIN_WIDTH = 900;
        const int DEF_WIN_HEIGHT = 450;
        const double aspectRatio = (0.5 * DEF_WIN_WIDTH) / DEF_WIN_HEIGHT;

        osg::ref_ptr <osg::GraphicsContext::Traits> traits =
                                            new osg::GraphicsContext::Traits;
        traits->x = DEF_WIN_X;
        traits->y = DEF_WIN_Y;
        traits->width = DEF_WIN_WIDTH;
        traits->height = DEF_WIN_HEIGHT;
        traits->windowDecoration = true;
        traits->doubleBuffer = true;
        traits->sharedContext = 0;
        traits->samples = 16;


        osg::ref_ptr <osg::GraphicsContext> gc =
                osg::GraphicsContext::createGraphicsContext(traits.get());
        if (gc.valid()) {
            osg::notify(osg::INFO) <<
            "  GraphicsWindow has been created successfully." << std::endl;

            // need to ensure that the window is cleared make sure that the
            // complete window is set the correct colour
            // rather than just the parts of the window that are under
            // the camera's viewports
            gc->setClearColor(osg::Vec4f(0.3f, 0.5f, 0.6f, 1.0f));
            gc->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
        else {
            osg::notify(osg::NOTICE) <<
            "  GraphicsWindow has not been created successfully." << std::endl;
        }

        // view one
        {
            osgViewer::View *view = new osgViewer::View;
            view->setName("Primitives View");
            viewer.addView(view);

            view->setSceneData(osg::get_pointer(primitiveRoot));
            view->getCamera()->setName("Primitives cam");
            view->getCamera()->setViewport(
                    new osg::Viewport(0, 0, traits->width / 2, traits->height));
            view->getCamera()->setGraphicsContext(gc.get());
            view->setCameraManipulator(new osgGA::TrackballManipulator);
            if (computeRayInFragmentShader) {
                osg::ref_ptr <osg::Uniform> vp = new osg::Uniform("viewport",
                                                                  osg::Vec4());
                primitiveRoot->getOrCreateStateSet()->addUniform(
                        osg::get_pointer(vp));
                view->getCamera()->setPreDrawCallback(
                        new CameraAspectRatioCBack(osg::get_pointer(vp)));
            }
            else
                view->getCamera()->setPreDrawCallback(
                        new CameraAspectRatioCBack);
            view->addEventHandler(new osgViewer::WindowSizeHandler);

        }

        osg::Camera *compositeViewCamera = 0;
        // view two
        {
            osgViewer::View *view = new osgViewer::View;
            view->setName("Composite view");
            viewer.addView(view);

            if (depthCamera != 0) {
                osg::ref_ptr <osg::Group> g = new osg::Group;
                g->getOrCreateStateSet()->addUniform(
                        new osg::Uniform("ssao", 1));
                //g->addChild( osg::get_pointer( depthCamera ) );
                g->addChild(osg::get_pointer(compositeRoot));
                view->setSceneData(osg::get_pointer(g));
                depthCamera->setGraphicsContext(gc.get());
                depthCamera->setViewport(
                        new osg::Viewport(0, 0, traits->width / 2,
                                          traits->height));
                depthCamera->getOrCreateStateSet()->addUniform(
                        new osg::Uniform("ssao", 0),
                        osg::StateAttribute::OVERRIDE);
            }
            else view->setSceneData(osg::get_pointer(compositeRoot));
            view->getCamera()->setName("Composite camera");
            view->getCamera()->setViewport(
                    new osg::Viewport(traits->width / 2, 0, traits->width / 2,
                                      traits->height));
            view->getCamera()->setGraphicsContext(gc.get());
            view->setCameraManipulator(new osgGA::TrackballManipulator);
            view->addEventHandler(new osgViewer::WindowSizeHandler);
            compositeViewCamera = view->getCamera();
            // Needed !! ??; needs more investigation to understand why so
            compositeViewCamera->setComputeNearFarMode(
                    osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
            if (computeRayInFragmentShader) {
                osg::ref_ptr <osg::Uniform> vp = new osg::Uniform("viewport",
                                                                  osg::Vec4());
                compositeRoot->getOrCreateStateSet()->addUniform(
                        osg::get_pointer(vp));
                if (!compositeBoxes.empty()) {
                    view->getCamera()->setPreDrawCallback(
                            CreateOptimalCompositeCameraCallback(
                                    primitives,
                                    primitiveNodes,
                                    compositeBoxes,
                                    osg::get_pointer(compositeBox),
                                    compositeHalfBoxSizes,
                                    primitiveMatrices,
                                    osg::get_pointer(vp)));
                }
                else {
                    view->getCamera()->setPreDrawCallback(
                            CreateCompositeCameraCallback(
                                    primitives,
                                    osg::get_pointer(primitiveRoot),
                                    osg::get_pointer(compositeRoot),
                                    osg::get_pointer(compositeBox),
                                    osg::get_pointer(halfBoxSize),
                                    osg::get_pointer(boxCenter),
                                    osg::get_pointer(vp)));
                }
            }
            else {
                if (!compositeBoxes.empty()) {
                    view->getCamera()->setPreDrawCallback(
                            CreateOptimalCompositeCameraCallback(
                                    primitives,
                                    primitiveNodes,
                                    compositeBoxes,
                                    osg::get_pointer(compositeBox),
                                    compositeHalfBoxSizes,
                                    primitiveMatrices));
                }
                else {
                    view->getCamera()->setPreDrawCallback(
                            CreateCompositeCameraCallback(
                                    primitives,
                                    osg::get_pointer(primitiveRoot),
                                    osg::get_pointer(compositeRoot),
                                    osg::get_pointer(compositeBox),
                                    osg::get_pointer(halfBoxSize),
                                    osg::get_pointer(boxCenter)));
                }
            }
        }

        {
            struct SnapImage : public osg::Camera::DrawCallback {
                SnapImage(const std::string &filename) :
                        _filename(filename),
                        _snapImage(false) {
                    _image = new osg::Image;
                }

                virtual void operator()(osg::RenderInfo &renderInfo) const {

                    if (!_snapImage) return;

                    osg::notify(osg::NOTICE) << "Camera callback" << std::endl;

                    osg::Camera *camera = renderInfo.getCurrentCamera();
                    osg::Viewport *viewport = camera ? camera->getViewport()
                                                     : 0;

                    osg::notify(osg::NOTICE) << "Camera callback " << camera <<
                    " " << viewport << std::endl;

                    if (viewport && _image.valid()) {
                        _image->readPixels(int(viewport->x()),
                                           int(viewport->y()),
                                           int(viewport->width()),
                                           int(viewport->height()),
                                           GL_DEPTH_COMPONENT,
                                           GL_UNSIGNED_BYTE);
                        osgDB::writeImageFile(*_image, _filename);

                        osg::notify(osg::NOTICE) <<
                        "Taken screenshot, and written to '" << _filename <<
                        "'" << std::endl;
                    }

                    _snapImage = false;
                }

                std::string _filename;
                mutable bool _snapImage;
                mutable osg::ref_ptr <osg::Image> _image;
            };

            osgViewer::Viewer::Windows windows;
            viewer.getWindows(windows);
            if (windows.empty()) return 1;
            // set up cameras to rendering on the first window available.
            //depthCamera->setGraphicsContext(windows[0]);
            osgViewer::View *depthView = new osgViewer::View;
            depthView->setCamera(depthCamera.get());
            //depthCamera->setPostDrawCallback(
            //                          new SnapImage( "~/tmp/depth.rgb" ) );
            viewer.addView(depthView);
        }
        //5) Add pre-render callback in right view to update uniforms
        //   for primitive matrices

        if (!depthCamera) return viewer.run();
        else {
            /// *** RENDERING LOOP ***///

            // since the pre render camera needs to be synchronized with the
            // main camera we need to perform the synchronization before the
            // actual rendering takes place
            // Fixed by properly setting a pre-draw callback to update both
            // pre-render and main camera
            viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);
            viewer.setReleaseContextAtEndOfFrameHint(false);
            viewer.realize();
            SyncCameraNode sn(compositeViewCamera,
                              osg::get_pointer(depthCamera),
                              osg::get_pointer(depthCameraViewportUniform));
            while (!viewer.done()) {
                viewer.advance();
                viewer.eventTraversal();
                viewer.updateTraversal();
                sn.SyncCameras();
                viewer.renderingTraversals();
            }
            return 0;
        }
    }
    catch (const std::exception &e) {
        std::cerr << e.what();
        return 1;
    }
}