#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include <math.h>
#include <vector>
#include <algorithm>
#include <iterator>
#include <stdio.h>
#include <iostream>
#include <sstream>

#include "imgui_canvas.h"
#include <windows.h>
// --------------------------------------------------------------------------------------------------------------------
uint8_t ImGuiCanvasElement::nextIndex = 0;
// --------------------------------------------------------------------------------------------------------------------
const std::string clipToString(const ImGuiCanvasClip _clip)
{
    std::string clipStrings[(uint8_t)ImGuiCanvasClip::COUNT] = IMGUICANVASCLIP_LABELS;
    return clipStrings[(uint8_t)_clip];
}

const bool stringToClip(const std::string _clipString, ImGuiCanvasClip& _clip)
{
    std::string clipStrings[(uint8_t)ImGuiCanvasClip::COUNT] = IMGUICANVASCLIP_LABELS;
    auto itShapeClip = std::find(std::begin(clipStrings), std::end(clipStrings), _clipString);
    if (itShapeClip != std::end(clipStrings))
    {
        _clip = (ImGuiCanvasClip)(uint8_t)std::distance(std::begin(clipStrings), itShapeClip);
        return true;
    }
    return false;
}

const std::string elementTypeToString(const ImGuiCanvasElementType _elementType)
{
    static const std::array<std::string, static_cast<size_t>(ImGuiCanvasElementType::COUNT)> shapeTypeStrings = {
        "TButton",
        "TLabel",
        "TEdit",
        "TMemo",
        "TToggleBox",
        "TCheckBox"
    };

    if (static_cast<uint8_t>(_elementType) < static_cast<uint8_t>(ImGuiCanvasElementType::COUNT))
    {
        return shapeTypeStrings[static_cast<size_t>(_elementType)];
    }
    return "Unknown";
}

const bool stringToElementType(const std::string _shapeTypeString, ImGuiCanvasElementType& _shapeType)
{
    /*std::string shapeTypeStrings[(uint8_t)ImGuiCanvasElementType::COUNT] = IMGUICANVASELEMENTTYPE_LABELS;
    auto itShapeType = std::find(std::begin(shapeTypeStrings), std::end(shapeTypeStrings), _shapeTypeString);
    if (itShapeType != std::end(shapeTypeStrings))
    {
        _shapeType = (ImGuiCanvasElementType)(uint8_t)std::distance(std::begin(shapeTypeStrings), itShapeType);
        return true;
    }*/
    return false;
}
// ---- constructor ---------------------------------------------------------------------------------------------------
ImGuiCanvasElement::ImGuiCanvasElement(std::string _label, const ImVec2 _center, const ImVec2 _size, const ImGuiCanvasElementType _type, const std::initializer_list<float> _parameters, const ImGuiCanvasClip _clip, const bool _drawCtrls) : m_label(_label), m_index(ImGuiCanvasElement::nextIndex++), m_type(_type), m_center(_center), m_clip(_clip), m_drawCtrls(_drawCtrls), m_visible(true), size(_size)
{
    m_ctrlPoints.push_back(m_center.position + ImVec2(*(_parameters.begin()), 0));
    m_ctrlPoints.push_back(m_center.position + ImVec2(0, *(_parameters.begin() + 1)));

    predifineElement(m_type);
    return;
}
ImGuiCanvasElement::ImGuiCanvasElement(std::string _label, const ImVec2 _center, const ImVec2 _size, const ImGuiCanvasElementType _type, const ImVector<float> _parameters, const ImGuiCanvasClip _clip, const bool _drawCtrls) : m_label(_label), m_index(ImGuiCanvasElement::nextIndex++), m_type(_type), m_center(_center), m_clip(_clip), m_drawCtrls(_drawCtrls), m_visible(true), size(_size)
{
    m_ctrlPoints.push_back(m_center.position + ImVec2(_parameters[0], 0));
    m_ctrlPoints.push_back(m_center.position + ImVec2(0, _parameters[1]));

    predifineElement(m_type);
    return;
}
ImGuiCanvasElement::ImGuiCanvasElement(std::string _label, const ImVec2 _center, const ImVec2 _size, const ImGuiCanvasElementType _type, const ImVector<ImVec2> _ctrlPoints, const ImGuiCanvasClip _clip, const bool _drawCtrls) : m_label(_label), m_index(ImGuiCanvasElement::nextIndex++), m_type(_type), m_center(_center), m_clip(_clip), m_drawCtrls(_drawCtrls), m_visible(true), size(_size)
{
    for (ImVec2 ctrlPoint : _ctrlPoints)
        m_ctrlPoints.push_back(ctrlPoint);

    predifineElement(m_type);
}
ImGuiCanvasElement::ImGuiCanvasElement(const ImGuiCanvasElement& _source) : m_label(_source.getLabel()), m_index(_source.getIndex()), m_type(_source.getType()), m_center(_source.getCenter()), m_ctrlPoints(_source.getCtrlPoints()), m_clip(_source.getClip()), m_drawCtrls(_source.getDrawCtrls()), m_visible(_source.getVisible()), size(_source.size) { predifineElement(m_type); }

// ---- destructor ----------------------------------------------------------------------------------------------------
ImGuiCanvasElement::~ImGuiCanvasElement() {}

// ---- assignment operator -------------------------------------------------------------------------------------------
ImGuiCanvasElement& ImGuiCanvasElement::operator=(const ImGuiCanvasElement& _other)
{
    m_color = _other.getColor();
    m_label = _other.getLabel();
    m_type = _other.getType();
    m_index = ImGuiCanvasElement::nextIndex;
    m_center = _other.getCenter();
    m_ctrlPoints = _other.getCtrlPoints();
    m_clip = _other.getClip();
    m_drawCtrls = _other.getDrawCtrls();
    return *this;
}

void ImGuiCanvasElement::predifineElement(ImGuiCanvasElementType _type)
{
    attributes.clear();

    int range = 10000 - 1 + 1;
    int randNum = (rand() % range + 1);
    switch (_type)
    {
    case ImGuiCanvasElementType::TButton:
        name = "Button" + std::to_string(randNum); //Placeholder name
        attributes.push_back(ElementAttribute("Action", ""));
        attributes.push_back(ElementAttribute("Align", ""));
        attributes.push_back(ElementAttribute("Anchors", ""));
        attributes.push_back(ElementAttribute("AutoSize", "", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("BidiMode", ""));
        attributes.push_back(ElementAttribute("BorderSpacing", ""));
        attributes.push_back(ElementAttribute("Cancel", "", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("Caption", name));
        attributes.push_back(ElementAttribute("Color", ""));
        attributes.push_back(ElementAttribute("Constraints", ""));
        attributes.push_back(ElementAttribute("Cursor", ""));
        attributes.push_back(ElementAttribute("Default", "", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("FoubleBuffered", "true", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("DragCursor", ""));
        attributes.push_back(ElementAttribute("DragKind", ""));
        attributes.push_back(ElementAttribute("DragMode", ""));
        attributes.push_back(ElementAttribute("Enabled", "true", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("Font", ""));
        attributes.push_back(ElementAttribute("HelpContext", ""));
        attributes.push_back(ElementAttribute("HelpKeyword", ""));
        attributes.push_back(ElementAttribute("HelpType", ""));
        attributes.push_back(ElementAttribute("Hint", ""));
        attributes.push_back(ElementAttribute("ModalResult", ""));
        attributes.push_back(ElementAttribute("ParentBidiMode", "true", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("ParentDoubleBuffered", "true", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("ParentFont", "true", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("ParentShowHint", "true", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("PopupMenu", ""));
        attributes.push_back(ElementAttribute("ShowHint", "true", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("TabOrder", "", ImGuiCanvasAttributeValueType::Integer));
        attributes.push_back(ElementAttribute("TabStop", "true", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("Tag", "0", ImGuiCanvasAttributeValueType::Integer));
        attributes.push_back(ElementAttribute("Visible", "", ImGuiCanvasAttributeValueType::Boolean));
        break;
    case ImGuiCanvasElementType::TLabel:
        name = "Label" + std::to_string(randNum); //Placeholder name
        attributes.push_back(ElementAttribute("Align", ""));
        attributes.push_back(ElementAttribute("Alignment", ""));
        attributes.push_back(ElementAttribute("Anchors", ""));
        attributes.push_back(ElementAttribute("AutoSize", "true", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("BidiMode", ""));
        attributes.push_back(ElementAttribute("BorderSpacing", ""));
        attributes.push_back(ElementAttribute("Caption", name));
        attributes.push_back(ElementAttribute("Color", ""));
        attributes.push_back(ElementAttribute("Constraints", ""));
        attributes.push_back(ElementAttribute("Cursor", ""));
        attributes.push_back(ElementAttribute("DragCursor", ""));
        attributes.push_back(ElementAttribute("DragKind", ""));
        attributes.push_back(ElementAttribute("DragMode", ""));
        attributes.push_back(ElementAttribute("Enabled", "true", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("FocusControl", ""));
        attributes.push_back(ElementAttribute("Font", ""));
        attributes.push_back(ElementAttribute("HelpContext", "0", ImGuiCanvasAttributeValueType::Integer));
        attributes.push_back(ElementAttribute("HelpKeyword", ""));
        attributes.push_back(ElementAttribute("HelpType", ""));
        attributes.push_back(ElementAttribute("Hint", ""));
        attributes.push_back(ElementAttribute("Layout", ""));
        attributes.push_back(ElementAttribute("OptimalFill", "", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("ParentBidiMode", "true", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("ParentColor", "true", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("ParentFont", "true", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("ParentShowHint", "true", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("Popup", ""));
        attributes.push_back(ElementAttribute("ShowAccelChar", "true", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("ShowHint", "", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("Tag", "0", ImGuiCanvasAttributeValueType::Integer));
        attributes.push_back(ElementAttribute("Transparent", "true", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("Visible", "true", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("WordWrap", "true", ImGuiCanvasAttributeValueType::Boolean));
        break;
    case ImGuiCanvasElementType::TEdit:
        name = "Edit" + std::to_string(randNum); //Placeholder name
        attributes.push_back(ElementAttribute("Align", ""));
        attributes.push_back(ElementAttribute("Alignment", ""));
        attributes.push_back(ElementAttribute("Anchors", ""));
        attributes.push_back(ElementAttribute("AutoSize", "true", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("AutoSelect", "true", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("BidiMode", ""));
        attributes.push_back(ElementAttribute("BorderSpacing", ""));
        attributes.push_back(ElementAttribute("BorderStyle", ""));
        attributes.push_back(ElementAttribute("CharCase", ""));
        attributes.push_back(ElementAttribute("Color", ""));
        attributes.push_back(ElementAttribute("Constraints", ""));
        attributes.push_back(ElementAttribute("DoubleBuffered", "true", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("DragCursor", ""));
        attributes.push_back(ElementAttribute("DragMode", ""));
        attributes.push_back(ElementAttribute("EchoMode", ""));
        attributes.push_back(ElementAttribute("Enabled", "true", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("Font", ""));
        attributes.push_back(ElementAttribute("HelpContext", "", ImGuiCanvasAttributeValueType::Integer));
        attributes.push_back(ElementAttribute("HelpKeyword", ""));
        attributes.push_back(ElementAttribute("HideSelection", "true", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("Hint", ""));
        attributes.push_back(ElementAttribute("MaxLength", "", ImGuiCanvasAttributeValueType::Integer));
        attributes.push_back(ElementAttribute("NumbersOnly", "", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("ParentBidiMode", "true", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("ParentColor", "", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("ParentDoubleBuffered", "true", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("ParentFont", "true", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("ParentShowHint", "true", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("PasswordChar", "#0"));
        attributes.push_back(ElementAttribute("PopupMenu", ""));
        attributes.push_back(ElementAttribute("ReadOnly", "", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("ShowHint", "", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("TabStop", "", ImGuiCanvasAttributeValueType::Boolean));
        attributes.push_back(ElementAttribute("TabOrder", "0", ImGuiCanvasAttributeValueType::Integer));
        attributes.push_back(ElementAttribute("Text", ""));
        attributes.push_back(ElementAttribute("TextHint", ""));
        attributes.push_back(ElementAttribute("Visible", "", ImGuiCanvasAttributeValueType::Boolean));
        break;
    case ImGuiCanvasElementType::TMemo:
        name = "Memo" + std::to_string(randNum); //Placeholder name
        break;
    case ImGuiCanvasElementType::TToggleBox:
        name = "ToggleBox" + std::to_string(randNum); //Placeholder name
        break;
    case ImGuiCanvasElementType::TCheckBox:
        name = "CheckBox" + std::to_string(randNum); //Placeholder name
        break;
    default:
        break;
    }
}

// --------------------------------------------------------------------------------------------------------------------
std::string ImGuiCanvasElement::toString() const
{
    std::stringstream _out;
    _out << getLabel() << "_" << getIndex() << " : (" << getCenter().position.x << "," << getCenter().position.y << ")";
    if (m_ctrlPoints.size() > 0)
    {
        _out << " {";
        for (const CtrlPoint& ctrlPoint : m_ctrlPoints)
            _out << "(" << ctrlPoint.position.x << ", " << ctrlPoint.position.y << ")";
        _out << "}";
    }
    _out << " " << ((bool)m_clip ? "In" : "Out");
    _out << " " << (m_drawCtrls ? "True" : "False");
    return _out.str();
}

// --------------------------------------------------------------------------------------------------------------------
float ImGuiCanvasElement::getAngle(uint8_t _index) const
{
    return ImGui::angle(m_center.position, m_ctrlPoints[_index].position);
}

// --------------------------------------------------------------------------------------------------------------------
float ImGuiCanvasElement::getAbsRadius(uint8_t _index) const
{
    return ImGui::length(m_center.position, m_ctrlPoints[_index].position);
}

// --------------------------------------------------------------------------------------------------------------------
ImVec2 ImGuiCanvasElement::getRadius(uint8_t _index) const
{
    return m_center.position - m_ctrlPoints[_index].position;
}

// --------------------------------------------------------------------------------------------------------------------
void ImGuiCanvasElement::MoveCenter(const ImVec2& _delta)
{
    if (getSelected())
    {
        m_center.move(_delta);
        for (int ctrlPointIndex = 0; ctrlPointIndex < m_ctrlPoints.size(); ctrlPointIndex++)
            m_ctrlPoints[ctrlPointIndex].move(_delta);
    }
}

// --------------------------------------------------------------------------------------------------------------------
void ImGuiCanvasElement::MoveCenter(const ImVec2& _delta, const ImVec2& _canvasSize)
{
    ImVec2 center, delta;
    if (getSelected())
    {
        // ----------------------------------------------------------------------------------------------------------------
        center = m_center.position;
        m_center.move(_delta);
        m_center.moveTo(ImClamp(m_center.position, { 0,0 }, _canvasSize));
        delta = m_center.position - center;
        // ----------------------------------------------------------------------------------------------------------------
        for (int ctrlPointIndex = 0; ctrlPointIndex < m_ctrlPoints.size(); ctrlPointIndex++)
            m_ctrlPoints[ctrlPointIndex].move(delta);
    }
}

// --------------------------------------------------------------------------------------------------------------------
void ImGuiCanvasElement::MoveCtrlPoint(int _ctrlPointIndex, const ImVec2& _delta)
{
    float rectangle_radius, rectangle_angle;
    if (m_ctrlPoints[_ctrlPointIndex].selected)
    {
        m_ctrlPoints[_ctrlPointIndex].move(_delta);
        if (_ctrlPointIndex == 0)
        {
            rectangle_radius = getAbsRadius(1);
            rectangle_angle = getAngle(0) + IM_PI / 2;
            m_ctrlPoints[1].position = getCenter().position + ImVec2(rectangle_radius * ImCos(rectangle_angle), rectangle_radius * ImSin(rectangle_angle));
        }
        else
        {
            rectangle_radius = getAbsRadius(0);
            rectangle_angle = getAngle(1) - IM_PI / 2;
            m_ctrlPoints[0].position = getCenter().position + ImVec2(rectangle_radius * ImCos(rectangle_angle), rectangle_radius * ImSin(rectangle_angle));
        }
    }
}

// --------------------------------------------------------------------------------------------------------------------
void ImGuiCanvasElement::MoveCtrlPoint(int _ctrlPointIndex, const ImVec2& _delta, const ImVec2& _canvasSize)
{
    ImVec2 point, delta;
    float rectangle_radius, rectangle_angle;
    if (m_ctrlPoints[_ctrlPointIndex].selected)
    {
        // ----------------------------------------------------------------------------------------------------------------
        point = m_ctrlPoints[_ctrlPointIndex].position;
        m_ctrlPoints[_ctrlPointIndex].move(_delta);
        m_ctrlPoints[_ctrlPointIndex].moveTo(ImClamp(m_ctrlPoints[_ctrlPointIndex].position, { 0,0 }, _canvasSize));
        delta = m_ctrlPoints[_ctrlPointIndex].position - point;
        // ----------------------------------------------------------------------------------------------------------------
        m_ctrlPoints[_ctrlPointIndex].move(delta);
        if (_ctrlPointIndex == 0)
        {
            rectangle_radius = getAbsRadius(1);
            rectangle_angle = getAngle(0) + IM_PI / 2;
            m_ctrlPoints[1].position = getCenter().position + ImVec2(rectangle_radius * ImCos(rectangle_angle), rectangle_radius * ImSin(rectangle_angle));
        }
        else
        {
            rectangle_radius = getAbsRadius(0);
            rectangle_angle = getAngle(1) - IM_PI / 2;
            m_ctrlPoints[0].position = getCenter().position + ImVec2(rectangle_radius * ImCos(rectangle_angle), rectangle_radius * ImSin(rectangle_angle));
        }
    }
}

// --------------------------------------------------------------------------------------------------------------------
bool ImGuiCanvasElement::isInside(const ImVec2& point) const
{
    float r0, a0, a;
    ImVec2 d0;
    r0 = ImGui::length(m_center.position, point);
    a0 = ImGui::angle(m_center.position, point);
    a = -getAngle();
    return applyClip(abs(r0 * ImCos(a + a0)), getAbsRadius(0), abs(r0 * ImSin(a + a0)), getAbsRadius(1));
}
// ====================================================================================================================
float ImGui::length(const ImVec2& a, const ImVec2& b)
{
    return ImGui::length(a - b);
}
// --------------------------------------------------------------------------------------------------------------------
float ImGui::length(const ImVec2& a)
{
    return ImSqrt(ImLengthSqr(a));
}
// --------------------------------------------------------------------------------------------------------------------
float ImGui::angle(const ImVec2& a, const ImVec2& b)
{
    return ImGui::angle(b - a);
}
// --------------------------------------------------------------------------------------------------------------------
float ImGui::angle(const ImVec2& a)
{
    return ImAtan2(a.y, a.x);
}
// --------------------------------------------------------------------------------------------------------------------
bool ImGui::DrawElements(const char* _label, const ImVec2& _origin, const ImVec2& _canvasSize, std::vector<ImGuiCanvasElement>& _shapes, const float _scale, const ImVec2& _size)
{
    static bool centerModified = false, ctrlPointModified = false;
    ImVec2 centerDelta, ctrlPointDelta;

    static char centerLabel[128];
    static char ctrlPointLabel[128];
    static ImGuiCanvasElement::CtrlPoint ctrlPoint;
    static ImVec2 transformedCenter, transformedCtrlPoint;
    static float transformedRadius;
    static float angle;
    static float xrate = 0.5, xmin = 0, xmax = _canvasSize.x;
    static float yrate = 0.5, ymin = 0, ymax = _canvasSize.y;
    static float xrateSize = 0.5, xminSize = 0, xmaxSize = _canvasSize.x;
    static float yrateSize = 0.5, yminSize = 0, ymaxSize = _canvasSize.y;
    static char centerXLabel[128], centerYLabel[128];
    static ImVec2 previous, next;

    auto vector_transform = [&](const ImVec2& _point) -> ImVec2 {return _origin + _point * _scale; };
    auto scalar_transform = [&](const float& _value) -> float {return _value * _scale; };

    ImGuiWindow* window = GetCurrentWindow();

    for (uint8_t shapeIndex = 0; shapeIndex < _shapes.size(); shapeIndex++)
    {

        if (!_shapes[shapeIndex].getVisible())
            continue;

        centerModified |= false;
        _shapes[shapeIndex].deselect();
        _shapes[shapeIndex].m_center.deselect();

        transformedCenter = vector_transform(ImClamp(_shapes[shapeIndex].m_center.position, { 0,0 }, _size));
        transformedRadius = scalar_transform(_shapes[shapeIndex].getAbsRadius());

        if (_shapes[shapeIndex].getDrawCtrls())
        {
            // ---- draw hot area for center control point ------------------------------------------------------------------
            sprintf_s(centerLabel, "shape#%s#%s#%04d#center", _label, _shapes[shapeIndex].getLabel().c_str(), shapeIndex);
            sprintf_s(centerXLabel, "##x#%s#%s#%04d#center", _label, _shapes[shapeIndex].getLabel().c_str(), shapeIndex);
            sprintf_s(centerYLabel, "##y#%s#%s#%04d#center", _label, _shapes[shapeIndex].getLabel().c_str(), shapeIndex);
            SetCursorScreenPos(transformedCenter - HANDLE_2);
            InvisibleButton(centerLabel, HANDLE_4);

            // ---- center control point hover ------------------------------------------------------------------------------
            _shapes[shapeIndex].m_center.setHovered(IsItemActive() || IsItemHovered());
            _shapes[shapeIndex].setHovered(IsItemActive() || IsItemHovered());
            // ---- center control point click ------------------------------------------------------------------------------
            if (IsItemActive() || IsItemClicked())
            {
                for (uint8_t shapeIndex = 0; shapeIndex < _shapes.size(); shapeIndex++)
                    _shapes[shapeIndex].deselect();
                _shapes[shapeIndex].m_center.select();
                _shapes[shapeIndex].select();
            }

            if (BeginPopupContextItem(centerLabel, ImGuiPopupFlags_MouseButtonRight))
            {
                previous = _shapes[shapeIndex].m_center.position;
                next = _shapes[shapeIndex].m_center.position;
                PushItemWidth(40);
                SameLine();
                if (DragScalar(centerXLabel, ImGuiDataType_Float, &next.x, xrate, &xmin, &xmax, "%.0f"))
                {
                    centerDelta = previous - next;
                    _shapes[shapeIndex].select();
                    _shapes[shapeIndex].MoveCenter(centerDelta, _size);
                    centerModified |= true;
                    _shapes[shapeIndex].deselect();
                };
                SameLine();
                if (DragScalar(centerYLabel, ImGuiDataType_Float, &next.y, yrate, &ymin, &ymax, "%.0f"))
                {
                    centerDelta = previous - next;
                    _shapes[shapeIndex].select();
                    _shapes[shapeIndex].MoveCenter(centerDelta, _size);
                    centerModified |= true;
                    _shapes[shapeIndex].deselect();
                }
                PopItemWidth();
                EndPopup();
            }

            if (_shapes[shapeIndex].m_center.getSelected() && IsMouseDragging(0))
            {
                centerDelta = GetIO().MouseDelta / _scale;
                _shapes[shapeIndex].MoveCenter(centerDelta, _size);
                centerModified |= true;
            }

            // ---- draw center control point -------------------------------------------------------------------------------
            window->DrawList->AddCircle(transformedCenter, MARK_4, _shapes[shapeIndex].m_center.getColor(), 32);
            // --------------------------------------------------------------------------------------------------------------
        }

        //Draw shape
        ImVec2 topLeft = ImVec2(transformedCenter.x - (_shapes[shapeIndex].size.x / 2), transformedCenter.y - (_shapes[shapeIndex].size.y / 2));
        ImVec2 bottomRight = ImVec2(topLeft.x + _shapes[shapeIndex].size.x, topLeft.y + _shapes[shapeIndex].size.y);
        window->DrawList->AddRect(topLeft, bottomRight, IM_COL32(200, 100, 100, 255));

        //Check for input
        if (IsKeyPressed(ImGuiKey_RightArrow))
        {
            // right arrow moves hovered center to the right 
            if (_shapes[shapeIndex].getHovered())
            {
                centerDelta = ImVec2(1, 0) / _scale;
                _shapes[shapeIndex].select();
                _shapes[shapeIndex].MoveCenter(centerDelta, _size);
                centerModified |= true;
                _shapes[shapeIndex].deselect();
            }
        }

        if (IsKeyPressed(ImGuiKey_UpArrow))
        {
            // up arrow moves hovered center to the up 
            if (_shapes[shapeIndex].getHovered())
            {
                centerDelta = ImVec2(0, -1) / _scale;
                _shapes[shapeIndex].select();
                _shapes[shapeIndex].MoveCenter(centerDelta, _size);
                centerModified |= true;
                _shapes[shapeIndex].deselect();
            }
        }

        if (IsKeyPressed(ImGuiKey_LeftArrow))
        {
            // left arrow moves hovered center to the left
            if (_shapes[shapeIndex].getHovered())
            {
                centerDelta = ImVec2(-1, 0) / _scale;
                _shapes[shapeIndex].select();
                _shapes[shapeIndex].MoveCenter(centerDelta, _size);
                centerModified |= true;
                _shapes[shapeIndex].deselect();
            }
        }

        if (IsKeyPressed(ImGuiKey_DownArrow))
        {
            // down arrow moves hovered center to the down
            if (_shapes[shapeIndex].getHovered())
            {
                centerDelta = ImVec2(0, 1) / _scale;
                _shapes[shapeIndex].select();
                _shapes[shapeIndex].MoveCenter(centerDelta, _size);
                centerModified |= true;
                _shapes[shapeIndex].deselect();
            }
        }

        if (IsKeyPressed(ImGuiKey_Delete))
        {
            // delete arrow deletes the shape hovered center
            if (_shapes[shapeIndex].getHovered())
            {
                _shapes.erase(_shapes.begin() + shapeIndex);
                centerModified |= true;
            }
        }

    }
    return (centerModified | ctrlPointModified);
}
// --------------------------------------------------------------------------------------------------------------------
int ImGui::DrawCanvas(const char* _label, const ImVec2& _viewSize, const ImVec2& _canvasSize, std::vector<ImGuiCanvasElement>& _shapes, const ImColor& _borderColor, const ImColor& _bgColor, ImU32 _flags)
{
    // no background image
    // draw shapes
    // no mask updating

    bool modified = false;
    static size_t count = 0;
    float scale = _viewSize.x / _canvasSize.x;

    if (count != _shapes.size())
        modified = true;

    count = _shapes.size();

    ImGuiWindow* window = GetCurrentWindow();
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    if (window->SkipItems)
        return false;

    ImRect canvas(window->DC.CursorPos, window->DC.CursorPos + _viewSize);

    ItemSize(canvas);
    if (!ItemAdd(canvas, 0))
        return false;

    window->DrawList->AddRectFilled(canvas.Min, canvas.Max, ImU32(_bgColor));

    window->DrawList->AddRect(canvas.Min, canvas.Max, ImU32(_borderColor));

    // window->DrawList->PushClipRect(canvas.Min, canvas.Max); // ----------------------------------------------------------

    modified |= DrawElements(_label, canvas.Min, _canvasSize, _shapes, scale, _canvasSize);

    SetCursorScreenPos(canvas.Min);
    InvisibleButton(_label, canvas.GetSize());
    if (IsItemClicked())
    {
        for (uint8_t shapeIndex = 0; shapeIndex < _shapes.size(); shapeIndex++)
            _shapes[shapeIndex].deselect();
    }

    const float GRID_STEP = 64.0 * scale;
    for (float x = 0; x < _viewSize.x; x += GRID_STEP)
        window->DrawList->AddLine(ImVec2(canvas.Min.x + x, canvas.Min.y), ImVec2(canvas.Min.x + x, canvas.Max.y), GRAY);
    for (float y = 0; y < _viewSize.y; y += GRID_STEP)
        window->DrawList->AddLine(ImVec2(canvas.Min.x, canvas.Min.y + y), ImVec2(canvas.Max.x, canvas.Min.y + y), GRAY);

    // window->DrawList->PopClipRect(); // ---------------------------------------------------------------------------------

    window->DC.CursorPos = canvas.Max + style.FramePadding;

    return modified;
}
// --------------------------------------------------------------------------------------------------------------------
int ImGui::DrawCanvas(const char* _label, const ImVec2& _viewSize, const ImVec2& _canvasSize, std::vector<ImGuiCanvasElement>& _shapes, ImTextureID _textureId, const ImColor& _borderColor, const ImColor& _bgColor, ImU32 _flags)
{
    // draw background image
    // draw shapes
    // no mask updating

    bool modified = false;
    static size_t count = 0;
    float scale = _viewSize.x / _canvasSize.x;

    if (count != _shapes.size())
        modified = true;

    count = _shapes.size();

    ImGuiWindow* window = GetCurrentWindow();
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    if (window->SkipItems)
        return false;

    ImRect canvas(window->DC.CursorPos, window->DC.CursorPos + _viewSize);

    ItemSize(canvas);
    if (!ItemAdd(canvas, 0))
        return false;

    window->DrawList->AddImage(_textureId, canvas.Min, canvas.Max);

    window->DrawList->AddRect(canvas.Min, canvas.Max, ImU32(_borderColor));

    // window->DrawList->PushClipRect(canvas.Min, canvas.Max); // ----------------------------------------------------------

    modified |= DrawElements(_label, canvas.Min, _canvasSize, _shapes, scale, _canvasSize);

    SetCursorScreenPos(canvas.Min);
    InvisibleButton(_label, canvas.GetSize());
    if (IsItemClicked())
    {
        for (uint8_t shapeIndex = 0; shapeIndex < _shapes.size(); shapeIndex++)
            _shapes[shapeIndex].deselect();
    }

    const float GRID_STEP = 64.0 * scale;
    for (float x = 0; x < _viewSize.x; x += GRID_STEP)
        window->DrawList->AddLine(ImVec2(canvas.Min.x + x, canvas.Min.y), ImVec2(canvas.Min.x + x, canvas.Max.y), GRAY);
    for (float y = 0; y < _viewSize.y; y += GRID_STEP)
        window->DrawList->AddLine(ImVec2(canvas.Min.x, canvas.Min.y + y), ImVec2(canvas.Max.x, canvas.Min.y + y), GRAY);

    // window->DrawList->PopClipRect(); // ---------------------------------------------------------------------------------

    window->DC.CursorPos = canvas.Max + style.FramePadding;

    return modified;
}
// --------------------------------------------------------------------------------------------------------------------
int ImGui::DrawCanvas(const char* _label, const ImVec2& _viewSize, const ImVec2& _canvasSize, std::vector<ImGuiCanvasElement>& _shapes, ImTextureID _textureId, uint8_t* _mask, bool _updateMask, const ImColor& _borderColor, const ImColor& _bgColor, ImU32 _flags)
{
    // draw background image
    // draw shapes
    // draw updating mask

    bool modified = false;
    static size_t count = 0;
    float scale = _viewSize.x / _canvasSize.x;

    if (count != _shapes.size())
        modified = true;

    count = _shapes.size();

    ImGuiWindow* window = GetCurrentWindow();
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    if (window->SkipItems)
        return false;

    ImRect canvas(window->DC.CursorPos, window->DC.CursorPos + _viewSize);

    ItemSize(canvas);
    if (!ItemAdd(canvas, 0))
        return false;

    window->DrawList->AddImage(_textureId, canvas.Min, canvas.Max);

    window->DrawList->AddRect(canvas.Min, canvas.Max, ImU32(_borderColor));

    // window->DrawList->PushClipRect(canvas.Min, canvas.Max); // ----------------------------------------------------------

    modified |= DrawElements(_label, canvas.Min, _canvasSize, _shapes, scale, _canvasSize);

    modified |= _updateMask;

    if (modified)
        UpdateMask(_mask, _canvasSize, _shapes);

    modified = false;

    SetCursorScreenPos(canvas.Min);
    InvisibleButton(_label, canvas.GetSize());
    if (IsItemClicked())
    {
        for (uint8_t shapeIndex = 0; shapeIndex < _shapes.size(); shapeIndex++)
            _shapes[shapeIndex].deselect();
    }

    const float GRID_STEP = 64.0 * scale;
    for (float x = 0; x < _viewSize.x; x += GRID_STEP)
        window->DrawList->AddLine(ImVec2(canvas.Min.x + x, canvas.Min.y), ImVec2(canvas.Min.x + x, canvas.Max.y), GRAY);
    for (float y = 0; y < _viewSize.y; y += GRID_STEP)
        window->DrawList->AddLine(ImVec2(canvas.Min.x, canvas.Min.y + y), ImVec2(canvas.Max.x, canvas.Min.y + y), GRAY);

    // window->DrawList->PopClipRect(); // ---------------------------------------------------------------------------------

    window->DC.CursorPos = canvas.Max + style.FramePadding;

    return modified;
}
// --------------------------------------------------------------------------------------------------------------------
int ImGui::DrawCanvas(const char* _label, const ImVec2& _viewSize, const ImVec2& _canvasSize, ImTextureID _textureId, const ImColor& _borderColor, const ImColor& _bgColor, ImU32 _flags)
{
    // draw background image
    // no shapes
    // no updating mask

    bool modified = false;
    float scale = _viewSize.x / _canvasSize.x;


    ImGuiWindow* window = GetCurrentWindow();
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    if (window->SkipItems)
        return false;

    ImRect canvas(window->DC.CursorPos, window->DC.CursorPos + _viewSize);

    ItemSize(canvas);
    if (!ItemAdd(canvas, 0))
        return false;

    window->DrawList->AddImage(_textureId, canvas.Min, canvas.Max);

    window->DrawList->AddRect(canvas.Min, canvas.Max, ImU32(_borderColor));

    // window->DrawList->PushClipRect(canvas.Min, canvas.Max); // ----------------------------------------------------------

    modified = false;

    const float GRID_STEP = 64.0 * scale;
    for (float x = 0; x < _viewSize.x; x += GRID_STEP)
        window->DrawList->AddLine(ImVec2(canvas.Min.x + x, canvas.Min.y), ImVec2(canvas.Min.x + x, canvas.Max.y), GRAY);
    for (float y = 0; y < _viewSize.y; y += GRID_STEP)
        window->DrawList->AddLine(ImVec2(canvas.Min.x, canvas.Min.y + y), ImVec2(canvas.Max.x, canvas.Min.y + y), GRAY);

    // window->DrawList->PopClipRect(); // ---------------------------------------------------------------------------------

    window->DC.CursorPos = canvas.Max + style.FramePadding;

    return modified;
}
// --------------------------------------------------------------------------------------------------------------------
void ImGui::UpdateMask(uint8_t* _mask, const ImVec2& _canvasSize, std::vector<ImGuiCanvasElement>& _shapes)
{
    for (int x = 0; x < _canvasSize.x; x++)
    {
        for (int y = 0; y < _canvasSize.y; y++)
        {
            _mask[x + (int)(_canvasSize.x * y)] = 0;
            for (ImGuiCanvasElement& shape : _shapes)
            {
                if (!shape.getVisible())
                    continue;
                _mask[x + (int)(_canvasSize.x * y)] |= (int)shape.isInside(ImVec2(x, y));
            }
        }
    }
}
// --------------------------------------------------------------------------------------------------------------------
void ImGui::AddCircle(ImDrawList* _ptrDrawList, const ImVec2& _center, float _radius, float _tilt, const ImColor& _color, int _numSegments, float _thickness)
{
    _ptrDrawList->AddCircle(_center, _radius, _color, _numSegments, _thickness);

    float IM_PI_2 = IM_PI / 2;
    ImVec2 marks[4];
    marks[0] = _center + ImVec2(_radius * ImCos(_tilt), _radius * ImSin(_tilt));
    _ptrDrawList->AddLine(_center, marks[0], _color, _thickness);
    marks[1] = _center + ImVec2(_radius * ImCos(IM_PI_2 + _tilt), _radius * ImSin(IM_PI_2 + _tilt));
    _ptrDrawList->AddLine(_center, marks[1], _color, _thickness);
    marks[2] = _center + ImVec2(_radius * ImCos(IM_PI + _tilt), _radius * ImSin(IM_PI + _tilt));
    _ptrDrawList->AddLine(_center, marks[2], _color, _thickness);
    marks[3] = _center + ImVec2(_radius * ImCos(3 * IM_PI_2 + _tilt), _radius * ImSin(3 * IM_PI_2 + _tilt));
    _ptrDrawList->AddLine(_center, marks[3], _color, _thickness);
}
// --------------------------------------------------------------------------------------------------------------------
void ImGui::AddRectangle(ImDrawList* _ptrDrawList, const ImVec2& _center, const ImVec2& _sideA, const ImVec2& _sideB, float _tilt, const ImColor& _color, float _thickness)
{
    ImVec2 points[4];
    points[0] = _center + ImVec2(_sideA.x - _sideB.x, _sideA.y - _sideB.y);
    points[1] = _center + ImVec2(-_sideA.x - _sideB.x, -_sideA.y - _sideB.y);
    points[2] = _center + ImVec2(-_sideA.x + _sideB.x, -_sideA.y + _sideB.y);
    points[3] = _center + ImVec2(_sideA.x + _sideB.x, +_sideA.y + _sideB.y);
    _ptrDrawList->AddQuad(points[0], points[1], points[2], points[3], _color);
}
// ====================================================================================================================