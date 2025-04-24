#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <string>
#include <vector>
#include <array>

#define IMGUI_DEFINE_MATH_OPERATORS

#define MARK_4 4
#define MARK_2 (MARK_4/2)

const ImVec2 HANDLE_2 = ImVec2(MARK_2, MARK_2);
const ImVec2 HANDLE_4 = ImVec2(MARK_4, MARK_4);
const ImColor WHITE = ImColor(255, 255, 255, 255);
const ImColor RED = ImColor(255, 0, 0, 255);
const ImColor YELLOW = ImColor(255, 255, 0, 255);
const ImColor GRAY = ImColor(200, 200, 200, 40);

#define BASE_VERTEX_SHADER_FILE_STR "shader/base.vertex.glsl"
#define GRAY_FRAGMENT_SHADER_FILE_STR "shader/gray.fragment.glsl"
#define MASK_FRAGMENT_SHADER_FILE_STR "shader/mask.fragment.glsl"

enum class ImGuiCanvasClip : uint8_t {
    In,
    Out,
    COUNT
};
#define IMGUICANVASCLIP_LABELS {"In",\
                                "Out"}

const std::string clipToString(const ImGuiCanvasClip _clip);
const bool stringToClip(const std::string _clipString, ImGuiCanvasClip& _clip);

enum class ImGuiCanvasElementType : uint8_t {
    TButton,
    TLabel,
    TEdit,
    TMemo,
    TToggleBox,
    TCheckBox,
    COUNT
};

enum class ImGuiCanvasAttributeValueType : uint8_t {
    String,
    Boolean,
    Integer,
    PredefinedAnswerList
};

const std::string elementTypeToString(const ImGuiCanvasElementType _elementType);
const bool stringToElementType(const std::string _shapeTypeString, ImGuiCanvasElementType& _elementType);

struct ElementAttribute 
{
    std::string attributeName;
    std::string attributeValue;
    ImGuiCanvasAttributeValueType attributeValueType;
    std::vector<std::string> enumTypes;

    ElementAttribute(std::string _attributeName, std::string _attributeValue, ImGuiCanvasAttributeValueType _attributeValueType = ImGuiCanvasAttributeValueType::String)
    {
        attributeName = _attributeName;
        attributeValue = _attributeValue;
        attributeValueType = _attributeValueType;
    }

    template <typename... Args>
    ElementAttribute(std::string _attributeName, std::string _attributeValue, Args&&... args)
    {
        attributeName = _attributeName;
        attributeValue = _attributeValue;
        attributeValueType = ImGuiCanvasAttributeValueType::PredefinedAnswerList;

        const auto variableParams = std::array<std::string_view, sizeof...(args)>{std::forward<Args>(args)...};
        if (not variableParams.empty())
        { //< Not needed because it's a nop to iterate over an empty array:
            for (const auto& item : variableParams)
            {
                enumTypes.push_back(item);
            }
        }
    }
};

struct ElementAction
{
    std::string actionName;
    std::string actionCustomName;

    ElementAction(std::string _actionName, std::string _actionCustomName)
    {
        actionName = _actionName;
        actionCustomName = _actionCustomName;
    }
};

class ImGuiCanvasElement {
public:
    struct CtrlPoint {
        ImVec2 position;
        ImVec2 size = HANDLE_4;
        bool selected = false, hovered = false;
        ImColor color = WHITE;
        // ----------------------------------------------------------------------------------------------------------------
        void setSelected(const bool _selected)
        {
            selected = _selected;
            color = (_selected) ? RED : WHITE;
        }
        bool getSelected() const
        {
            return selected;
        }
        void select()
        {
            setSelected(true);
        }
        void deselect()
        {
            setSelected(false);
        }
        // ----------------------------------------------------------------------------------------------------------------
        void setHovered(const bool _hovered)
        {
            hovered = _hovered;
            color = (_hovered) ? YELLOW : WHITE;
        }
        bool getHovered() const
        {
            return hovered;
        }
        void hover()
        {
            setHovered(true);
        }
        // ----------------------------------------------------------------------------------------------------------------
        ImColor getColor() const
        {
            return color;
        }
        // ----------------------------------------------------------------------------------------------------------------
        void move(const ImVec2& _delta)
        {
            position.x = position.x + _delta.x;
            position.y = position.y + _delta.y;
        }
        void moveTo(const ImVec2& _position)
        {
            position = _position;
        }
        // ----------------------------------------------------------------------------------------------------------------
        CtrlPoint(const ImVec2 _position) : position(_position) {}
        CtrlPoint(const float x, const float y) : position({ x,y }) {}
        CtrlPoint() = default;
    };
    // ---- constructor -------------------------------------------------------------------------------------------------
    ImGuiCanvasElement() = default;
    ImGuiCanvasElement(std::string _label, const ImVec2 _center, const ImVec2 _size, const ImGuiCanvasElementType _type, const std::initializer_list<float> _parameters, const ImGuiCanvasClip _clip, const bool _drawCtrls = true);
    ImGuiCanvasElement(std::string _label, const ImVec2 _center, const ImVec2 _size, const ImGuiCanvasElementType _type, const ImVector<float> _parameters, const ImGuiCanvasClip _clip, const bool _drawCtrls = true);
    ImGuiCanvasElement(std::string _label, const ImVec2 _center, const ImVec2 _size, const ImGuiCanvasElementType _type, const ImVector<ImVec2> _ctrlPoints, const ImGuiCanvasClip _clip, const bool _drawCtrls = true);
    ImGuiCanvasElement(const ImGuiCanvasElement& _source); // copy constructor

    // ---- destructor --------------------------------------------------------------------------------------------------
    ~ImGuiCanvasElement();

    // ---- assignment operator -----------------------------------------------------------------------------------------
    ImGuiCanvasElement& operator=(const ImGuiCanvasElement& _other);

    // ------------------------------------------------------------------------------------------------------------------
    void predifineElement(ImGuiCanvasElementType _type);
    
    // ------------------------------------------------------------------------------------------------------------------
    void setSelected(const bool _selected)
    {
        m_selected = _selected;
        m_color = (_selected) ? RED : WHITE;
    }
    bool getSelected() const
    {
        return m_selected;
    }
    void select()
    {
        setSelected(true);
    }
    void deselect()
    {
        setSelected(false);
    }
    // ------------------------------------------------------------------------------------------------------------------
    void setHovered(const bool _hovered)
    {
        m_hovered = _hovered;
        m_color = (_hovered) ? YELLOW : WHITE;
    }
    bool getHovered() const
    {
        return m_hovered;
    }
    void hover()
    {
        setHovered(true);
    }
    // ------------------------------------------------------------------------------------------------------------------
    void setVisible(const bool _visible = true)
    {
        m_visible = _visible;
    }
    bool getVisible() const
    {
        return m_visible;
    }
    // ------------------------------------------------------------------------------------------------------------------
    CtrlPoint getCenter() const
    {
        return m_center;
    }
    CtrlPoint& getRefCenter()
    {
        return m_center;
    }
    std::string getLabel() const
    {
        return m_label;
    }
    bool isLabel(const std::string _label) const
    {
        return (m_label == _label);
    }
    uint8_t getIndex() const
    {
        return m_index;
    }
    ImVector<CtrlPoint> getCtrlPoints() const
    {
        return m_ctrlPoints;
    }
    CtrlPoint getCtrlPoint(uint8_t _index = 0) const
    {
        return m_ctrlPoints[_index];
    }
    CtrlPoint& getRefCtrlPoint(uint8_t _index = 0)
    {
        return m_ctrlPoints[_index];
    }
    ImGuiCanvasElementType getType() const
    {
        return m_type;
    }
    ImColor getColor() const
    {
        return m_color;
    }
    ImGuiCanvasClip getClip() const
    {
        return m_clip;
    }
    std::string toString() const;
    float getAngle(uint8_t _index = 0) const;
    ImVec2 getRadius(uint8_t _index = 0) const;
    float getAbsRadius(uint8_t _index = 0) const;

    void setCtrlPoint(CtrlPoint _ctrlPoint, uint8_t _index = 0)
    {
        m_ctrlPoints[_index] = _ctrlPoint;
    }

    bool getCtrlPointSelected(uint8_t _index) const
    {
        return m_ctrlPoints[_index].getSelected();
    }
    void setCtrlPointSelected(uint8_t _index, const bool _selected)
    {
        m_ctrlPoints[_index].setSelected(_selected);
    }
    void selectCtrlPoint(uint8_t _index)
    {
        m_ctrlPoints[_index].select();
    }
    void deselectCtrlPoint(uint8_t _index)
    {
        m_ctrlPoints[_index].deselect();
    }

    bool getCtrlPointHovered(uint8_t _index) const
    {
        return m_ctrlPoints[_index].getHovered();
    }
    void setCtrlPointHovered(uint8_t _index, const bool _hovered)
    {
        m_ctrlPoints[_index].setHovered(_hovered);
    }
    void hoverCtrlPoint(uint8_t _index = 0)
    {
        m_ctrlPoints[_index].hover();
    }
    void setDrawCtrls(const bool _drawCtrls)
    {
        m_drawCtrls = _drawCtrls;
    }
    bool getDrawCtrls() const
    {
        return m_drawCtrls;
    }
    inline bool applyClip(float a, float b) const
    {
        return (bool)m_clip ? (a > b) : (a < b);
    }
    inline bool applyClip(float a1, float b1, float a2, float b2) const
    {
        return (bool)m_clip ? ((a1 < b1) && (a2 < b2)) : ((a1 > b1) || (a2 > b2));
    }
    void MoveCenter(const ImVec2& _delta);
    void MoveCenter(const ImVec2& _delta, const ImVec2& _canvasSize);
    void MoveCtrlPoint(int _ctrlPointIndex, const ImVec2& _delta);
    void MoveCtrlPoint(int _ctrlPointIndex, const ImVec2& _delta, const ImVec2& _canvasSize);
    bool isInside(const ImVec2& point) const;

    ImVec2 size;
    std::string name;
    std::vector<ElementAttribute> attributes;
    std::vector<ElementAction> actions;
private:
    std::string m_label;
    uint8_t m_index;
    ImGuiCanvasElementType m_type;
public:
    CtrlPoint m_center;
    ImVector<CtrlPoint> m_ctrlPoints;
private:
    ImGuiCanvasClip m_clip = ImGuiCanvasClip::In;
    bool m_drawCtrls = true;
    static uint8_t nextIndex;
    ImColor m_color = WHITE;
    bool m_selected = false, m_hovered = false, m_visible = true;
};
// ====================================================================================================================
namespace ImGui
{
    float length(const ImVec2& a);
    float length(const ImVec2& a, const ImVec2& b);
    float angle(const ImVec2& a);
    float angle(const ImVec2& a, const ImVec2& b);
    bool DrawElements(const char* _label, const ImVec2& _origin, const ImVec2& _canvasSize, std::vector<ImGuiCanvasElement>& _elements, const float _scale = 1.0, const ImVec2& _size = ImVec2(0, 0));
    void UpdateMask(uint8_t* _mask, const ImVec2& _canvasSize, std::vector<ImGuiCanvasElement>& _elements);

    IMGUI_API int DrawCanvas(const char* _label, const ImVec2& _viewSize, const ImVec2& _canvasSize, std::vector<ImGuiCanvasElement>& _elements, const ImColor& _borderColor = WHITE, const ImColor& _bgColor = ImColor(50, 50, 50, 255), ImU32 _flags = 0);
    IMGUI_API int DrawCanvas(const char* _label, const ImVec2& _viewSize, const ImVec2& _canvasSize, std::vector<ImGuiCanvasElement>& _elements, ImTextureID _textureId, const ImColor& _borderColor = WHITE, const ImColor& _bgColor = ImColor(50, 50, 50, 255), ImU32 _flags = 0);
    IMGUI_API int DrawCanvas(const char* _label, const ImVec2& _viewSize, const ImVec2& _canvasSize, std::vector<ImGuiCanvasElement>& _elements, ImTextureID _textureId, uint8_t* mask, bool _updateMask = false, const ImColor& _borderColor = WHITE, const ImColor& _bgColor = ImColor(50, 50, 50, 255), ImU32 _flags = 0);
    IMGUI_API int DrawCanvas(const char* _label, const ImVec2& _viewSize, const ImVec2& _canvasSize, ImTextureID _textureId, const ImColor& _borderColor = WHITE, const ImColor& _bgColor = ImColor(50, 50, 50, 255), ImU32 _flags = 0);

    IMGUI_API void AddCircle(ImDrawList* _ptrDrawList, const ImVec2& _center, float _radius, float _tilt, const ImColor& _color = WHITE, int _numSegments = 32, float _thickness = 1.0f);
    IMGUI_API void AddRectangle(ImDrawList* _ptrDrawList, const ImVec2& _center, const ImVec2& _sideA, const ImVec2& _sideB, float _tilt, const ImColor& _color = WHITE, float _thickness = 1.0f);

}
// ====================================================================================================================