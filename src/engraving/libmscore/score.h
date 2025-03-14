/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __SCORE_H__
#define __SCORE_H__

/**
 \file
 Definition of Score class.
*/

#include <set>
#include <memory>

#include "async/channel.h"
#include "types/ret.h"
#include "compat/midi/midirender.h"

#include "modularity/ioc.h"
#include "draw/iimageprovider.h"
#include "iengravingfontsprovider.h"

#include "types/constants.h"

#include "layout/ilayout.h"
#include "layout/layoutoptions.h"

#include "style/style.h"
#include "style/pagestyle.h"

#include "chordlist.h"
#include "input.h"
#include "mscore.h"
#include "property.h"
#include "scoreorder.h"
#include "select.h"
#include "spannermap.h"
#include "synthesizerstate.h"
#include "rootitem.h"

namespace mu::engraving {
class IMimeData;
}

namespace mu::engraving::read114 {
class Read114;
}

namespace mu::engraving::read302 {
class Read302;
}

namespace mu::engraving::read400 {
class Read400;
}

namespace mu::engraving::read410 {
class Read410;
}

namespace mu::engraving::write {
class Writer;
}

namespace mu::engraving::compat {
class WriteScoreHook;
}

namespace mu::engraving::layout::v0 {
class ScoreLayout;
}

namespace mu::engraving {
class Articulation;
class Audio;
class Box;
class Bracket;
class Chord;
class ChordRest;
class Clef;
class Element;
class EventMap;
class Excerpt;
class FiguredBass;
class Hairpin;
class Harmony;
class InstrumentTemplate;
class InputState;
class KeyList;
class KeySigEvent;
class LinkedObjects;
class Lyrics;
class MasterScore;
class Measure;
class MeasureBase;
class MuseScoreView;
class Note;
class Page;
class Part;
class RehearsalMark;
class RepeatList;
class Rest;
class Score;
class ScoreElement;
class IEngravingFont;
class Segment;
class Slur;
class Spanner;
class Staff;
class System;
class TDuration;
class TempoMap;
class Text;
class TimeSig;
class TimeSigMap;
class Tuplet;
class UndoCommand;
class UndoStack;

class ShadowNote;

struct Interval;
struct NoteVal;

enum class BeatType : char;
enum class Key;
enum class HairpinType : signed char;
enum class SegmentType;
enum class OttavaType : char;
enum class Voicing : signed char;
enum class HDuration : signed char;
enum class AccidentalType;
enum class LayoutBreakType;

enum class POS : char {
    CURRENT, LEFT, RIGHT
};

enum class Pad : char {
    NOTE00,
    NOTE0,
    NOTE1,
    NOTE2,
    NOTE4,
    NOTE8,
    NOTE16,
    NOTE32,
    NOTE64,
    NOTE128,
    NOTE256,
    NOTE512,
    NOTE1024,
    //--------------------
    REST,
    DOT,
    DOT2,
    DOT3,
    DOT4
};

//---------------------------------------------------------
//   MeasureBaseList
//---------------------------------------------------------

class MeasureBaseList
{
    int _size;
    MeasureBase* _first = nullptr;
    MeasureBase* _last = nullptr;

    void push_back(MeasureBase* e);
    void push_front(MeasureBase* e);

public:
    MeasureBaseList();
    MeasureBase* first() const { return _first; }
    MeasureBase* last()  const { return _last; }
    void clear() { _first = _last = 0; _size = 0; }
    void add(MeasureBase*);
    void remove(MeasureBase*);
    void insert(MeasureBase*, MeasureBase*);
    void remove(MeasureBase*, MeasureBase*);
    void change(MeasureBase* o, MeasureBase* n);
    int size() const { return _size; }
    bool empty() const { return _size == 0; }
};

//---------------------------------------------------------
//   MidiInputEvent
//---------------------------------------------------------

struct MidiInputEvent {
    int pitch;
    bool chord;
    int velocity;
};

//---------------------------------------------------------
//   Position
//---------------------------------------------------------

struct Position {
    Segment* segment { nullptr };
    staff_idx_t staffIdx = mu::nidx;
    int line         { 0 };
    int fret         { INVALID_FRET_INDEX };
    mu::PointF pos;
};

//---------------------------------------------------------
//   LayoutFlag bits
//---------------------------------------------------------

enum class LayoutFlag : char {
    NO_FLAGS       = 0,
    FIX_PITCH_VELO = 1,
    PLAY_EVENTS    = 2,
    REBUILD_MIDI_MAPPING = 4,
};

typedef Flags<LayoutFlag> LayoutFlags;

//---------------------------------------------------------
//   PlayMode
//---------------------------------------------------------

enum class PlayMode : char {
    SYNTHESIZER,
    AUDIO
};

//---------------------------------------------------------
//   UpdateMode
//    There is an implied order from least invasive update
//    to most invasive update. LayoutAll is fallback and
//    recreates all.
//---------------------------------------------------------

enum class UpdateMode {
    DoNothing,
    Update,             // do screen refresh of RectF "refresh"
    UpdateAll,          // do complete screen refresh
    Layout,             // do partial layout for tick range
};

//---------------------------------------------------------
//   CmdState
//
//    the following variables are reset on startCmd()
//    modified during cmd processing and used in endCmd() to
//    determine what to layout and what to repaint:
//---------------------------------------------------------

class CmdState
{
    UpdateMode _updateMode { UpdateMode::DoNothing };
    Fraction _startTick { -1, 1 };            // start tick for mode LayoutTick
    Fraction _endTick   { -1, 1 };              // end tick for mode LayoutTick
    staff_idx_t _startStaff = mu::nidx;
    staff_idx_t _endStaff = mu::nidx;
    const EngravingItem* _el = nullptr;
    const MeasureBase* _mb = nullptr;
    bool _oneElement = true;
    bool _oneMeasureBase = true;

    bool _locked = false;

    void setMeasureBase(const MeasureBase* mb);

public:
    LayoutFlags layoutFlags;

    bool _excerptsChanged     { false };
    bool _instrumentsChanged  { false };

    void reset();
    UpdateMode updateMode() const { return _updateMode; }
    void setUpdateMode(UpdateMode m);
    void _setUpdateMode(UpdateMode m);
    bool layoutRange() const { return _updateMode == UpdateMode::Layout; }
    bool updateAll() const { return int(_updateMode) >= int(UpdateMode::UpdateAll); }
    bool updateRange() const { return _updateMode == UpdateMode::Update; }
    void setTick(const Fraction& t);
    void setStaff(staff_idx_t staff);
    void setElement(const EngravingItem* e);
    void unsetElement(const EngravingItem* e);
    Fraction startTick() const { return _startTick; }
    Fraction endTick() const { return _endTick; }
    staff_idx_t startStaff() const { return _startStaff; }
    staff_idx_t endStaff() const { return _endStaff; }
    const EngravingItem* element() const;

    void lock() { _locked = true; }
    void unlock() { _locked = false; }
#ifndef NDEBUG
    void dump();
#endif
};

//---------------------------------------------------------
//   UpdateState
//---------------------------------------------------------

class UpdateState
{
public:
    mu::RectF refresh;                 ///< area to update, canvas coordinates
    bool _playNote   { false };     ///< play selected note after command
    bool _playChord  { false };     ///< play whole chord for the selected note
    bool _selectionChanged { false };
    std::list<EngravingObject*> _deleteList;
};

//---------------------------------------------------------
//   PaddingTable
//---------------------------------------------------------

template<typename T>
struct PaddingVector : std::array<T, TOT_ELEMENT_TYPES>
{
    T& operator [](size_t i) { return std::array<T, TOT_ELEMENT_TYPES>::operator [](i); }
    T& operator [](ElementType et) { return std::array<T, TOT_ELEMENT_TYPES>::operator [](static_cast<size_t>(et)); }
    const T& at(ElementType et) const { return std::array<T, TOT_ELEMENT_TYPES>::at(static_cast<size_t>(et)); }
};
using PaddingTable = PaddingVector<PaddingVector<double> >;

//---------------------------------------------------------------------------------------
//   @@ Score
//   @P composer        string            composer of the score (read only)
//   @P duration        int               duration of score in seconds (read only)
//   @P excerpts        array[Excerpt]    the list of the excerpts (linked parts)
//   @P firstMeasure    Measure           the first measure of the score (read only)
//   @P firstMeasureMM  Measure           the first multi-measure rest measure of the score (read only)
//   @P harmonyCount    int               number of harmony items (read only)
//   @P hasHarmonies    bool              score has chord symbols (read only)
//   @P hasLyrics       bool              score has lyrics (read only)
//   @P keysig          int               key signature at the start of the score (read only)
//   @P lastMeasure     Measure           the last measure of the score (read only)
//   @P lastMeasureMM   Measure           the last multi-measure rest measure of the score (read only)
//   @P lastSegment     Segment           the last score segment (read-only)
//   @P lyricCount      int               number of lyric items (read only)
//   @P name            string            name of the score
//   @P nmeasures       int               number of measures (read only)
//   @P npages          int               number of pages (read only)
//   @P nstaves         int               number of staves (read only)
//   @P ntracks         int               number of tracks (staves * 4) (read only)
// not to be documented?
//   @P parts           array[Part]       the list of parts (read only)
//
//    a Score has always an associated MasterScore
//---------------------------------------------------------------------------------------

class Score : public EngravingObject
{
    OBJECT_ALLOCATOR(engraving, Score)
    DECLARE_CLASSOF(ElementType::SCORE)

    INJECT(draw::IImageProvider, imageProvider)
    INJECT(IEngravingConfiguration, configuration)
    INJECT(IEngravingFontsProvider, engravingFonts)

    // internal
    INJECT(layout::ILayout, layout)

private:

    friend class read302::Read302;
    friend class read400::Read400;
    friend class read410::Read410;
    friend class write::Writer;
    friend class layout::v0::ScoreLayout;

    static std::set<Score*> validScores;
    int _linkId { 0 };
    MasterScore* _masterScore { 0 };
    std::list<MuseScoreView*> viewer;
    Excerpt* _excerpt  { 0 };

    std::vector<Text*> _headersText;
    std::vector<Text*> _footersText;

    String _mscoreVersion;
    int _mscoreRevision;

    std::shared_ptr<IEngravingFont> m_engravingFont = nullptr;
    int _pageNumberOffset { 0 };          ///< Offset for page numbers.

    UpdateState _updateState;

    MeasureBaseList _measures;            // here are the notes
    std::vector<Part*> _parts;
    std::vector<Staff*> _staves;
    std::vector<Staff*> m_systemObjectStaves;

    SpannerMap _spanner;
    std::set<Spanner*> _unmanagedSpanner;

    //
    // objects generated by layout:
    //
    std::vector<Page*> _pages;            // pages are build from systems
    std::vector<System*> _systems;        // measures are accumulated to systems

    InputState _is;
    MStyle m_style;
    ChordList _chordList;

    bool _showInvisible         { true };
    bool _showUnprintable       { true };
    bool _showFrames            { true };
    bool _showPageborders       { false };
    bool _markIrregularMeasures { true };
    bool _showInstrumentNames   { true };
    bool _printing              { false };        ///< True if we are drawing to a printer
    bool _savedCapture          { false };        ///< True if we saved an image capture

    ScoreOrder _scoreOrder;                     ///< used for score ordering
    bool _resetAutoplace = false;
    bool _resetDefaults = false;
    int _mscVersion = Constants::MSC_VERSION;     ///< version of current loading *.msc file

    bool _isOpen { false };
    bool _needSetUpTempoMap { true };

    std::map<String, String> _metaTags;

    Selection _selection;
    SelectionFilter _selectionFilter;
    Audio* _audio { nullptr };
    PlayMode _playMode { PlayMode::SYNTHESIZER };

    RootItem* m_rootItem = nullptr;
    LayoutOptions m_layoutOptions;

    double m_noteHeadWidth = 0.0;

    mu::async::Channel<EngravingItem*> m_elementDestroyed;

    ShadowNote* m_shadowNote = nullptr;

    mu::async::Channel<POS, unsigned> m_posChanged;

    ScoreChangesRange changesRange() const;

    Note* getSelectedNote();
    ChordRest* nextTrack(ChordRest* cr, bool skipMeasureRepeatRests = true);
    ChordRest* prevTrack(ChordRest* cr, bool skipMeasureRepeatRests = true);

    void addTempo();
    void addMetronome();

    void checkSlurs();
    void checkScore();

    bool rewriteMeasures(Measure* fm, Measure* lm, const Fraction&, staff_idx_t staffIdx);
    bool rewriteMeasures(Measure* fm, const Fraction& ns, staff_idx_t staffIdx);
    std::list<Fraction> splitGapToMeasureBoundaries(ChordRest*, Fraction);
    void pasteChordRest(ChordRest* cr, const Fraction& tick);

    void doSelect(EngravingItem* e, SelectType type, staff_idx_t staffIdx);
    void selectSingle(EngravingItem* e, staff_idx_t staffIdx);
    void selectAdd(EngravingItem* e);
    void selectRange(EngravingItem* e, staff_idx_t staffIdx);

    Ret putNote(const Position&, bool replace);

    void resetTempo();
    void resetTempoRange(const Fraction& tick1, const Fraction& tick2);
    void rebuildTempoAndTimeSigMaps(Measure* m);

    void deleteSpannersFromRange(const Fraction& t1, const Fraction& t2, track_idx_t trackStart, track_idx_t trackEnd,
                                 const SelectionFilter& filter);
    void deleteAnnotationsFromRange(Segment* segStart, Segment* segEnd, track_idx_t trackStart, track_idx_t trackEnd,
                                    const SelectionFilter& filter);
    ChordRest* deleteRange(Segment* segStart, Segment* segEnd, track_idx_t trackStart, track_idx_t trackEnd, const SelectionFilter& filter);

    void update(bool resetCmdState, bool layoutAllParts = false);

    ID newStaffId() const;
    ID newPartId() const;

    void assignIdIfNeed(Staff& staff) const;
    void assignIdIfNeed(Part& part) const;

    void updateStavesNumberForSystems();

    PaddingTable _paddingTable;
    double _minimumPaddingUnit = 0.1 * spatium(); // Maybe style setting in future
    bool _updatesLocked = false;

protected:
    int _fileDivision;   ///< division of current loading *.msc file
    SynthesizerState _synthesizerState;

    void createPlayEvents(Chord* chord, Chord* prevChord = nullptr);
    void createGraceNotesPlayEvents(const Fraction& tick, Chord* chord, int& ontime, int& trailtime);
    void cmdPitchUp();
    void cmdPitchDown();
    void cmdPitchUpOctave();
    void cmdPitchDownOctave();

    friend class MasterScore;
    Score();
    Score(MasterScore*, bool forcePartStyle = true);
    Score(MasterScore*, const MStyle&);

public:
    Score(const Score&) = delete;
    Score& operator=(const Score&) = delete;
    virtual ~Score();
    Score* clone();

    static Score* paletteScore();
    bool isPaletteScore() const;

    virtual bool isMaster() const { return false; }
    virtual bool readOnly() const;

    static void onElementDestruction(EngravingItem* se);

    // Score Tree functions
    EngravingObject* scanParent() const override;
    EngravingObjectList scanChildren() const override;
    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    void dumpScoreTree();  // for debugging purposes

    RootItem* rootItem() const { return m_rootItem; }
    compat::DummyElement* dummy() const { return m_rootItem->dummy(); }

    ShadowNote& shadowNote() const;

    mu::async::Channel<POS, unsigned> posChanged() const;
    void notifyPosChanged(POS pos, unsigned ticks);

    mu::async::Channel<EngravingItem*> elementDestroyed();

    void rebuildBspTree();
    bool noStaves() const { return _staves.empty(); }
    void insertPart(Part*, size_t targetPartIdx);
    void appendPart(Part*);
    void removePart(Part*);
    void insertStaff(Staff*, staff_idx_t);
    void appendStaff(Staff*);
    void cmdRemoveStaff(staff_idx_t staffIdx);
    void removeStaff(Staff*);
    void addMeasure(MeasureBase*, MeasureBase*);
    void linkMeasures(Score* score);
    void setResetAutoplace() { _resetAutoplace = true; }
    void setResetDefaults() { _resetDefaults = true; }

    Excerpt* excerpt() { return _excerpt; }
    void setExcerpt(Excerpt* e) { _excerpt = e; }

    // methods for resetting elements for pre-4.0 score migration
    void resetAutoplace();
    void resetDefaults();

    void cmdAddBracket();
    void cmdAddParentheses();
    void cmdAddBraces();
    void cmdAddFret(int fret);
    void cmdSetBeamMode(BeamMode);
    void cmdRemovePart(Part*);
    void cmdAddTie(bool addToChord = false);
    void cmdToggleTie();
    static std::vector<Note*> cmdTieNoteList(const Selection& selection, bool noteEntryMode);
    void cmdAddOttava(OttavaType);
    std::vector<Hairpin*> addHairpins(HairpinType);
    void addNoteLine();
    void padToggle(Pad p, const EditData& ed);
    void cmdAddPitch(const EditData&, int note, bool addFlag, bool insert);
    void cmdAddStretch(double);
    void cmdAddGrace(NoteType, int);
    void cmdResetNoteAndRestGroupings();
    void cmdResetAllPositions(bool undoable = true);
    void cmdDoubleDuration() { cmdIncDecDuration(-1, false); }
    void cmdHalfDuration() { cmdIncDecDuration(1, false); }
    void cmdIncDurationDotted() { cmdIncDecDuration(-1, true); }
    void cmdDecDurationDotted() { cmdIncDecDuration(1, true); }
    void cmdIncDecDuration(int nSteps, bool stepDotted = false);
    void cmdToggleLayoutBreak(LayoutBreakType);
    void cmdAddStaffTypeChange(Measure* measure, staff_idx_t staffIdx, StaffTypeChange* stc);
    void cmdAddMeasureRepeat(Measure*, int numMeasures, staff_idx_t staffIdx);
    bool makeMeasureRepeatGroup(Measure*, int numMeasures, staff_idx_t staffIdx);
    void cmdFlip();
    void resetUserStretch();
    void cmdResetBeamMode();
    void cmdResetTextStyleOverrides();
    void cmdInsertClef(ClefType);
    void removeChordRest(ChordRest* cr, bool clearSegment);
    ChordRest* nextMeasure(ChordRest* element, bool selectBehavior = false, bool mmRest = false);
    ChordRest* prevMeasure(ChordRest* element, bool mmRest = false);
    ChordRest* upStaff(ChordRest* cr);
    ChordRest* downStaff(ChordRest* cr);
    void cmdPadNoteIncreaseTAB(const EditData& ed);
    void cmdPadNoteDecreaseTAB(const EditData& ed);
    void cmdToggleMmrest();
    void cmdToggleHideEmpty();
    void cmdSetVisible();
    void cmdUnsetVisible();
    void cmdMoveRest(Rest*, DirectionV);
    void cmdMoveLyrics(Lyrics*, DirectionV);

    void realtimeAdvance();

    void addRemoveBreaks(int interval, bool lock);

    bool transpose(Note* n, Interval, bool useSharpsFlats);
    void transposeKeys(staff_idx_t staffStart, staff_idx_t staffEnd, const Fraction& tickStart, const Fraction& tickEnd, bool flip = false);
    bool transpose(TransposeMode mode, TransposeDirection, Key transposeKey, int transposeInterval, bool trKeys, bool transposeChordNames,
                   bool useDoubleSharpsFlats);

    bool appendMeasuresFromScore(Score* score, const Fraction& startTick, const Fraction& endTick);
    bool appendScore(Score*, bool addPageBreak = false, bool addSectionBreak = true);

    const std::vector<Staff*>& staves() const { return _staves; }
    size_t nstaves() const { return _staves.size(); }
    size_t ntracks() const { return _staves.size() * VOICES; }

    staff_idx_t staffIdx(const Staff*) const;
    staff_idx_t staffIdx(const Part*) const;
    Staff* staff(size_t n) const { return (n < _staves.size()) ? _staves.at(n) : nullptr; }
    Staff* staffById(const ID& staffId) const;
    Part* partById(const ID& partId) const;

    void clearSystemObjectStaves();
    void addSystemObjectStaff(Staff* staff);
    const std::vector<Staff*>& systemObjectStaves() const { return m_systemObjectStaves; }
    bool isSystemObjectStaff(Staff* staff) const;

    Measure* pos2measure(const mu::PointF&, staff_idx_t* staffIdx, int* pitch, Segment**, mu::PointF* offset) const;
    void dragPosition(const mu::PointF&, staff_idx_t* staffIdx, Segment**, double spacingFactor = 0.5) const;

    void undoAddElement(EngravingItem* element, bool addToLinkedStaves = true, bool ctrlModifier = false);
    void undoAddCR(ChordRest* element, Measure*, const Fraction& tick);
    void undoRemoveElement(EngravingItem* element);
    void undoChangeSpannerElements(Spanner* spanner, EngravingItem* startElement, EngravingItem* endElement);
    void undoChangeElement(EngravingItem* oldElement, EngravingItem* newElement);
    void undoChangePitch(Note* note, int pitch, int tpc1, int tpc2);
    void undoChangeFretting(Note* note, int pitch, int string, int fret, int tpc1, int tpc2);
    void spellNotelist(std::vector<Note*>& notes);
    void undoChangeTpc(Note* note, int tpc);
    void undoChangeChordRestLen(ChordRest* cr, const TDuration&);
    void undoTransposeHarmony(Harmony*, int, int);
    void undoExchangeVoice(Measure* measure, voice_idx_t val1, voice_idx_t val2, staff_idx_t staff1, staff_idx_t staff2);
    void undoRemovePart(Part* part, size_t partIdx = mu::nidx);
    void undoInsertPart(Part* part, size_t targetPartIndex);
    void undoRemoveStaff(Staff* staff);
    void undoInsertStaff(Staff* staff, staff_idx_t idx, bool createRests=true);
    void undoChangeVisible(EngravingItem* item, bool visible);
    void undoChangeTuning(Note*, double);
    void undoChangeUserMirror(Note*, DirectionH);
    void undoChangeKeySig(Staff* ostaff, const Fraction& tick, KeySigEvent);
    void undoChangeClef(Staff* ostaff, EngravingItem*, ClefType st, bool forInstrumentChange = false);
    bool undoPropertyChanged(EngravingItem* e, Pid t, const PropertyValue& st, PropertyFlags ps = PropertyFlags::NOSTYLE);
    void undoPropertyChanged(EngravingObject*, Pid, const PropertyValue& v, PropertyFlags ps = PropertyFlags::NOSTYLE);
    virtual UndoStack* undoStack() const;
    void undo(UndoCommand*, EditData* = nullptr) const;
    void undoRemoveMeasures(Measure*, Measure*, bool preserveTies = false);
    void undoChangeMeasureRepeatCount(Measure* m, int count, staff_idx_t staffIdx);
    void undoAddBracket(Staff* staff, size_t level, BracketType type, size_t span);
    void undoRemoveBracket(Bracket*);
    void undoInsertTime(const Fraction& tick, const Fraction& len);
    void undoChangeStyleVal(Sid idx, const PropertyValue& v);
    void undoChangePageNumberOffset(int po);

    void updateInstrumentChangeTranspositions(KeySigEvent& key, Staff* staff, const Fraction& tick);

    Note* setGraceNote(Chord*,  int pitch, NoteType type, int len);

    Segment* setNoteRest(Segment*, track_idx_t track, NoteVal nval, Fraction, DirectionV stemDirection = DirectionV::AUTO,
                         bool forceAccidental = false, const std::set<SymId>& articulationIds = {}, bool rhythmic = false,
                         InputState* externalInputState = nullptr);
    Segment* setChord(Segment*, track_idx_t track, const Chord* chordTemplate, Fraction, DirectionV stemDirection = DirectionV::AUTO);
    void changeCRlen(ChordRest* cr, const TDuration&);
    void changeCRlen(ChordRest* cr, const Fraction&, bool fillWithRest=true);
    void createCRSequence(const Fraction& f, ChordRest* cr, const Fraction& tick);

    Fraction makeGap(Segment*, track_idx_t track, const Fraction&, Tuplet*, bool keepChord = false);
    bool makeGap1(const Fraction& baseTick, staff_idx_t staffIdx, const Fraction& len, int voiceOffset[VOICES]);
    bool makeGapVoice(Segment* seg, track_idx_t track, Fraction len, const Fraction& tick);

    Rest* addRest(const Fraction& tick, track_idx_t track, TDuration, Tuplet*);
    Rest* addRest(Segment* seg, track_idx_t track, TDuration d, Tuplet*);
    Chord* addChord(const Fraction& tick, TDuration d, Chord* oc, bool genTie, Tuplet* tuplet);
    MeasureRepeat* addMeasureRepeat(const Fraction& tick, track_idx_t track, int numMeasures);

    Tuplet* addTuplet(ChordRest* destinationChordRest, Fraction ratio, TupletNumberType numberType, TupletBracketType bracketType);

    ChordRest* addClone(ChordRest* cr, const Fraction& tick, const TDuration& d);
    Rest* setRest(const Fraction& tick,  track_idx_t track, const Fraction&, bool useDots, Tuplet* tuplet, bool useFullMeasureRest = true);

    void upDown(bool up, UpDownMode);
    void upDownDelta(int pitchDelta);
    ChordRest* searchNote(const Fraction& tick, track_idx_t track) const;

    // undo/redo ops
    void toggleArticulation(SymId);
    bool toggleArticulation(EngravingItem*, Articulation* atr);
    void toggleAccidental(AccidentalType, const EditData& ed);
    void changeAccidental(AccidentalType);
    void changeAccidental(Note* oNote, AccidentalType);

    void addElement(EngravingItem*);
    void removeElement(EngravingItem*);
    bool containsElement(const EngravingItem*) const;

    Note* addPitch(NoteVal&, bool addFlag, InputState* externalInputState = nullptr);
    Note* addTiedMidiPitch(int pitch, bool addFlag, Chord* prevChord);
    NoteVal noteVal(int pitch) const;
    Note* addMidiPitch(int pitch, bool addFlag);
    Note* addNote(Chord*, const NoteVal& noteVal, bool forceAccidental = false, const std::set<SymId>& articulationIds = {},
                  InputState* externalInputState = nullptr);
    Note* addNoteToTiedChord(Chord*, const NoteVal& noteVal, bool forceAccidental = false, const std::set<SymId>& articulationIds = {});

    NoteVal noteValForPosition(Position pos, AccidentalType at, bool& error);

    Slur* addSlur(ChordRest* firstChordRest, ChordRest* secondChordRest, const Slur* slurTemplate);
    TextBase* addText(TextStyleType type, EngravingItem* destinationElement = nullptr, bool addToAllScores = true);

    void deleteItem(EngravingItem*);
    void deleteMeasures(MeasureBase* firstMeasure, MeasureBase* lastMeasure, bool preserveTies = false);
    void restoreInitialKeySig();
    void reconnectSlurs(MeasureBase* mbStart, MeasureBase* mbLast);
    void cmdDeleteSelection();
    void cmdFullMeasureRest();

    Ret putNote(const mu::PointF&, bool replace, bool insert);
    Ret insertChordByInsertingTime(const Position&);

    void cloneVoice(track_idx_t strack, track_idx_t dtrack, Segment* sf, const Fraction& lTick, bool link = true, bool spanner = true);

    Ret repitchNote(const Position& pos, bool replace);
    void regroupNotesAndRests(const Fraction& startTick, const Fraction& endTick, track_idx_t track);

    void startCmd();                    // start undoable command
    void endCmd(bool rollback = false, bool layoutAllParts = false); // end undoable command
    void update() { update(true); }
    void lockUpdates(bool locked);
    void undoRedo(bool undo, EditData*);

    virtual mu::async::Channel<ScoreChangesRange> changesChannel() const;

    void cmdRemoveTimeSig(TimeSig*);
    void cmdAddTimeSig(Measure*, staff_idx_t staffIdx, TimeSig*, bool local);

    virtual void setUpdateAll();
    void setLayoutAll(staff_idx_t staff = mu::nidx, const EngravingItem* e = nullptr);
    void setLayout(const Fraction& tick, staff_idx_t staff, const EngravingItem* e = nullptr);
    void setLayout(const Fraction& tick1, const Fraction& tick2, staff_idx_t staff1, staff_idx_t staff2, const EngravingItem* e = nullptr);
    virtual CmdState& cmdState();
    virtual const CmdState& cmdState() const;
    virtual void addLayoutFlags(LayoutFlags);
    virtual void setInstrumentsChanged(bool);
    void addRefresh(const mu::RectF&);

    void cmdToggleAutoplace(bool all);

    bool playNote() const { return _updateState._playNote; }
    void setPlayNote(bool v) { _updateState._playNote = v; }
    bool playChord() const { return _updateState._playChord; }
    void setPlayChord(bool v) { _updateState._playChord = v; }
    bool selectionEmpty() const { return _selection.staffStart() == _selection.staffEnd(); }
    bool selectionChanged() const { return _updateState._selectionChanged; }
    void setSelectionChanged(bool val) { _updateState._selectionChanged = val; }
    void deleteLater(EngravingObject* e) { _updateState._deleteList.push_back(e); }
    void deletePostponed();

    void changeSelectedNotesVoice(voice_idx_t);

    const std::vector<Part*>& parts() const;
    int visiblePartCount() const;

    using StaffAccepted = std::function<bool (const Staff&)>;
    std::set<staff_idx_t> staffIdxSetFromRange(const track_idx_t trackFrom, const track_idx_t trackTo,
                                               StaffAccepted staffAccepted = StaffAccepted()) const;

    void appendPart(const InstrumentTemplate*);
    void updateStaffIndex();
    void sortSystemObjects(std::vector<staff_idx_t>& dst);
    void sortStaves(std::vector<staff_idx_t>& dst);

    bool showInvisible() const { return _showInvisible; }
    bool showUnprintable() const { return _showUnprintable; }
    bool showFrames() const { return _showFrames; }
    bool showPageborders() const { return _showPageborders; }
    bool markIrregularMeasures() const { return _markIrregularMeasures; }
    bool showInstrumentNames() const { return _showInstrumentNames; }
    void setShowInvisible(bool v);
    void setShowUnprintable(bool v);
    void setShowFrames(bool v);
    void setShowPageborders(bool v);
    void setMarkIrregularMeasures(bool v);
    void setShowInstrumentNames(bool v) { _showInstrumentNames = v; }

    void print(mu::draw::Painter* printer, int page);
    ChordRest* getSelectedChordRest() const;
    std::set<ChordRest*> getSelectedChordRests() const;
    void getSelectedChordRest2(ChordRest** cr1, ChordRest** cr2) const;

    void select(EngravingItem* item, SelectType = SelectType::SINGLE, staff_idx_t staff = 0);
    void select(const std::vector<EngravingItem*>& items, SelectType = SelectType::SINGLE, staff_idx_t staff = 0);
    void selectSimilar(EngravingItem* e, bool sameStaff);
    void selectSimilarInRange(EngravingItem* e);
    static void collectMatch(void* data, EngravingItem* e);
    static void collectNoteMatch(void* data, EngravingItem* e);
    void deselect(EngravingItem* obj);
    void deselectAll() { _selection.deselectAll(); }
    void updateSelection() { _selection.update(); }
    EngravingItem* getSelectedElement() const { return _selection.element(); }
    const Selection& selection() const { return _selection; }
    Selection& selection() { return _selection; }
    SelectionFilter& selectionFilter() { return _selectionFilter; }
    void setSelection(const Selection& s);

    Fraction pos();
    Measure* tick2measure(const Fraction& tick) const;
    Measure* tick2measureMM(const Fraction& tick) const;
    MeasureBase* tick2measureBase(const Fraction& tick) const;
    Segment* tick2segment(const Fraction& tick, bool first, SegmentType st, bool useMMrest = false) const;
    Segment* tick2segment(const Fraction& tick) const;
    Segment* tick2segment(const Fraction& tick, bool first) const;
    Segment* tick2segmentMM(const Fraction& tick, bool first, SegmentType st) const;
    Segment* tick2segmentMM(const Fraction& tick) const;
    Segment* tick2segmentMM(const Fraction& tick, bool first) const;
    Segment* tick2leftSegment(const Fraction& tick, bool useMMrest = false) const;
    Segment* tick2rightSegment(const Fraction& tick, bool useMMrest = false) const;
    Segment* tick2leftSegmentMM(const Fraction& tick) { return tick2leftSegment(tick, /* useMMRest */ true); }

    void setUpTempoMapLater();
    void setUpTempoMap();

    EngravingItem* nextElement();
    EngravingItem* prevElement();
    ChordRest* cmdNextPrevSystem(ChordRest*, bool);
    Box* cmdNextPrevFrame(MeasureBase*, bool) const;
    EngravingItem* cmdNextPrevSection(EngravingItem*, bool) const;
    MeasureBase* getNextPrevSectionBreak(MeasureBase*, bool) const;
    EngravingItem* getScoreElementOfMeasureBase(MeasureBase*) const;

    int fileDivision(int t) const { return static_cast<int>(((int64_t)t * Constants::DIVISION + _fileDivision / 2) / _fileDivision); }
    void setFileDivision(int t) { _fileDivision = t; }

    bool dirty() const;
    bool savedCapture() const { return _savedCapture; }
    void setSavedCapture(bool v) { _savedCapture = v; }
    bool printing() const { return _printing; }
    void setPrinting(bool val) { _printing = val; }
    virtual bool playlistDirty() const;
    virtual void setPlaylistDirty();

    bool isOpen() const;
    void setIsOpen(bool open);

    void spell();
    void spell(staff_idx_t startStaff, staff_idx_t endStaff, Segment* startSegment, Segment* endSegment);
    void spell(Note*);
    void changeEnharmonicSpelling(bool both);

    Fraction nextSeg(const Fraction& tick, int track);

    ChordList* chordList() { return &_chordList; }
    const ChordList* chordList() const { return &_chordList; }
    void checkChordList() { _chordList.checkChordList(style()); }

    MStyle& style() { return m_style; }
    const MStyle& style() const { return m_style; }

    PageSizeGetAccessor pageSize() const { return PageSizeGetAccessor(m_style); }
    PageSizeSetAccessor pageSize() { return PageSizeSetAccessor(m_style); }

    void resetStyleValue(Sid styleToReset);

    void setStyle(const MStyle& s, const bool overlap = false);
    bool loadStyle(const String&, bool ign = false, const bool overlap = false);
    bool saveStyle(const String&);

    const PropertyValue& styleV(Sid idx) const { return style().styleV(idx); }
    Spatium  styleS(Sid idx) const { return style().styleS(idx); }
    Millimetre styleMM(Sid idx) const { return style().styleMM(idx); }
    String styleSt(Sid idx) const { return style().styleSt(idx); }
    bool styleB(Sid idx) const { return style().styleB(idx); }
    double styleD(Sid idx) const { return style().styleD(idx); }
    int styleI(Sid idx) const { return style().styleI(idx); }

    void setStyleValue(Sid sid, const PropertyValue& value) { style().set(sid, value); }
    TranslatableString getTextStyleUserName(TextStyleType tid);
    double spatium() const { return styleD(Sid::spatium); }
    void setSpatium(double v) { setStyleValue(Sid::spatium, v); }

    // These position are in ticks and not uticks
    Fraction playPos() const { return pos(POS::CURRENT); }
    void setPlayPos(const Fraction& tick) { setPos(POS::CURRENT, tick); }
    Fraction loopInTick() const { return pos(POS::LEFT); }
    Fraction loopOutTick() const { return pos(POS::RIGHT); }
    void setLoopInTick(const Fraction& tick) { setPos(POS::LEFT, tick); }
    void setLoopOutTick(const Fraction& tick) { setPos(POS::RIGHT, tick); }

    Fraction pos(POS pos) const;
    void setPos(POS pos, Fraction tick);

    bool noteEntryMode() const { return inputState().noteEntryMode(); }
    void setNoteEntryMode(bool val) { inputState().setNoteEntryMode(val); }
    NoteEntryMethod noteEntryMethod() const { return inputState().noteEntryMethod(); }
    void setNoteEntryMethod(NoteEntryMethod m) { inputState().setNoteEntryMethod(m); }
    bool usingNoteEntryMethod(NoteEntryMethod m) { return inputState().usingNoteEntryMethod(m); }
    Fraction inputPos() const;
    track_idx_t inputTrack() const { return inputState().track(); }
    const InputState& inputState() const { return _is; }
    InputState& inputState() { return _is; }
    void setInputState(const InputState& st) { _is = st; }
    void setInputTrack(int t) { inputState().setTrack(t); }

    void spatiumChanged(double oldValue, double newValue);
    void styleChanged() override;

    void cmdPaste(const IMimeData* ms, MuseScoreView* view, Fraction scale = Fraction(1, 1));
    bool pasteStaff(XmlReader&, Segment* dst, staff_idx_t staffIdx, Fraction scale = Fraction(1, 1));
    void pasteSymbols(XmlReader& e, ChordRest* dst);
    void renderMidi(EventMap* events, const MidiRenderer::Context& ctx, bool expandRepeats);

    static void transposeChord(Chord* c, const Fraction& tick);

    BeatType tick2beatType(const Fraction& tick) const;

    int mscVersion() const { return _mscVersion; }
    void setMscVersion(int v) { _mscVersion = v; }

    void addLyrics(const Fraction& tick, staff_idx_t staffIdx, const String&);

    void updateSwing();
    void createPlayEvents(Measure const* start = nullptr, Measure const* const end = nullptr);

    void updateCapo();
    void updateVelo();
    void updateChannel();

    void cmdConcertPitchChanged(bool);

    virtual TempoMap* tempomap() const;
    virtual TimeSigMap* sigmap() const;

    void setTempo(Segment*, BeatsPerSecond bps);
    void setTempo(const Fraction& tick, BeatsPerSecond bps);
    void removeTempo(const Fraction& tick);
    void setPause(const Fraction& tick, double seconds);
    BeatsPerSecond tempo(const Fraction& tick) const;

    Text* getText(TextStyleType subtype) const;

    bool enableVerticalSpread() const;
    void setEnableVerticalSpread(bool val);

    ScoreOrder scoreOrder() const;
    void setScoreOrder(ScoreOrder order);
    void updateBracesAndBarlines(Part* part, size_t index);
    void setBracketsAndBarlines();
    void remapBracketsAndBarlines();

    void lassoSelect(const mu::RectF&);
    void lassoSelectEnd();

    Page* searchPage(const mu::PointF&) const;
    std::vector<System*> searchSystem(const mu::PointF& p, const System* preferredSystem = nullptr, double spacingFactor = 0.5,
                                      double preferredSpacingFactor = 1.0) const;
    Measure* searchMeasure(const mu::PointF& p, const System* preferredSystem = nullptr, double spacingFactor = 0.5,
                           double preferredSpacingFactor = 1.0) const;

    bool getPosition(Position* pos, const mu::PointF&, voice_idx_t voice) const;

    void cmdDeleteTuplet(Tuplet*, bool replaceWithRest);
    Measure* getCreateMeasure(const Fraction& tick);

    void adjustBracketsDel(size_t sidx, size_t eidx);
    void adjustBracketsIns(size_t sidx, size_t eidx);
    void adjustKeySigs(track_idx_t sidx, track_idx_t eidx, KeyList km);
    KeyList keyList() const;

    /// To be used together with setExpandRepeats.
    /// For bigger operations, where suboperations might also use it,
    /// where those need to have the same value for expandRepeats.
    virtual const RepeatList& repeatList() const;
    /// For small, one-step operations, where you need to get the relevant repeatList just once
    virtual const RepeatList& repeatList(bool expandRepeats) const;

    double utick2utime(int tick) const;
    int utime2utick(double utime) const;

    void nextInputPos(ChordRest* cr, bool);
    void cmdMirrorNoteHead();

    virtual size_t npages() const { return _pages.size(); }
    virtual page_idx_t pageIdx(const Page* page) const { return mu::indexOf(_pages, page); }
    virtual const std::vector<Page*>& pages() const { return _pages; }
    virtual std::vector<Page*>& pages() { return _pages; }

    const std::vector<System*>& systems() const { return _systems; }
    std::vector<System*>& systems() { return _systems; }

    MeasureBaseList* measures() { return &_measures; }
    bool checkHasMeasures() const;
    MeasureBase* first() const;
    MeasureBase* firstMM() const;
    MeasureBase* last()  const;
    Measure* firstMeasure() const;
    Measure* firstMeasureMM() const;
    Measure* lastMeasure() const;
    Measure* lastMeasureMM() const;
    MeasureBase* measure(int idx) const;
    Measure* crMeasure(int idx) const;

    Fraction endTick() const;

    Segment* firstSegment(SegmentType s) const;
    Segment* firstSegmentMM(SegmentType s) const;
    Segment* lastSegment() const;
    Segment* lastSegmentMM() const;

    void connectTies(bool silent = false);

    double point(const Spatium sp) const { return sp.val() * spatium(); }

    void scanElementsInRange(void* data, void (* func)(void*, EngravingItem*), bool all = true);
    int fileDivision() const { return _fileDivision; }   ///< division of current loading *.msc file
    void splitStaff(staff_idx_t staffIdx, int splitPoint);
    Lyrics* addLyrics();
    FiguredBass* addFiguredBass();
    void expandVoice(Segment* s, track_idx_t track);
    void expandVoice();

    EngravingItem* selectMove(const String& cmd);
    EngravingItem* move(const String& cmd);
    void cmdEnterRest(const TDuration& d);
    void enterRest(const TDuration& d, InputState* externalInputState = nullptr);
    void addInterval(int, const std::vector<Note*>&);
    void cmdCreateTuplet(ChordRest*, Tuplet*);
    void removeAudio();

    bool autoLayoutEnabled() const;

    void doLayout();
    void doLayoutRange(const Fraction& st, const Fraction& et);

    SynthesizerState& synthesizerState() { return _synthesizerState; }
    void setSynthesizerState(const SynthesizerState& s);

    void updateHairpin(Hairpin*);         // add/modify hairpin to pitchOffset list

    MasterScore* masterScore() const { return _masterScore; }
    void setMasterScore(MasterScore* s) { _masterScore = s; }

    const std::map<String, String>& metaTags() const { return _metaTags; }
    std::map<String, String>& metaTags() { return _metaTags; }
    void setMetaTags(const std::map<String, String>& t) { _metaTags = t; }

    //@ returns as a string the metatag named 'tag'
    String metaTag(const String& tag) const;
    //@ sets the metatag named 'tag' to 'val'
    void setMetaTag(const String& tag, const String& val);

    void cmdSplitMeasure(ChordRest*);
    void splitMeasure(Segment*);
    void cmdJoinMeasure(Measure*, Measure*);

    int pageNumberOffset() const { return _pageNumberOffset; }
    void setPageNumberOffset(int v) { _pageNumberOffset = v; }

    String mscoreVersion() const { return _mscoreVersion; }
    int mscoreRevision() const { return _mscoreRevision; }
    void setMscoreVersion(const String& val) { _mscoreVersion = val; }
    void setMscoreRevision(int val) { _mscoreRevision = val; }

    void addViewer(MuseScoreView* v) { viewer.push_back(v); }
    void removeViewer(MuseScoreView* v) { viewer.remove(v); }
    const std::list<MuseScoreView*>& getViewer() const { return viewer; }

    //! NOTE Layout
    const LayoutOptions& layoutOptions() const { return m_layoutOptions; }
    void setLayoutMode(LayoutMode lm) { m_layoutOptions.mode = lm; }
    void setShowVBox(bool v) { m_layoutOptions.isShowVBox = v; }
    double noteHeadWidth() const { return m_noteHeadWidth; }
    void setNoteHeadWidth(double n) { m_noteHeadWidth = n; }

    // temporary methods
    bool isLayoutMode(LayoutMode lm) const { return m_layoutOptions.isMode(lm); }
    LayoutMode layoutMode() const { return m_layoutOptions.mode; }
    bool lineMode() const { return m_layoutOptions.isMode(LayoutMode::LINE); }
    bool linearMode() const { return m_layoutOptions.isLinearMode(); }
    // ----

    void cmdSelectAll();
    void cmdSelectSection();
    void transposeSemitone(int semitone);
    void transposeDiatonicAlterations(TransposeDirection direction);

    struct InsertMeasureOptions {
        InsertMeasureOptions() {}

        bool createEmptyMeasures = false;
        bool moveSignaturesClef = true;
        bool needDeselectAll = true;
        bool addToAllScores = true;
    };

    MeasureBase* insertMeasure(ElementType type, MeasureBase* beforeMeasure = nullptr,
                               const InsertMeasureOptions& options = InsertMeasureOptions());

    Audio* audio() const { return _audio; }
    void setAudio(Audio* a) { _audio = a; }
    PlayMode playMode() const { return _playMode; }
    void setPlayMode(PlayMode v) { _playMode = v; }

    int linkId();
    void linkId(int);
    int getLinkId() const { return _linkId; }

    std::list<Score*> scoreList();

    //@ appends to the score a number of measures
    void appendMeasures(int);

    const std::multimap<int, Spanner*>& spanner() const { return _spanner.map(); }
    SpannerMap& spannerMap() { return _spanner; }
    const SpannerMap& spannerMap() const { return _spanner; }
    bool isSpannerStartEnd(const Fraction& tick, track_idx_t track) const;
    void removeSpanner(Spanner*);
    void addSpanner(Spanner*);
    void cmdAddSpanner(Spanner* spanner, const mu::PointF& pos, bool systemStavesOnly = false);
    void cmdAddSpanner(Spanner* spanner, staff_idx_t staffIdx, Segment* startSegment, Segment* endSegment);
    void checkSpanner(const Fraction& startTick, const Fraction& lastTick, bool removeOrphans = true);
    const std::set<Spanner*> unmanagedSpanners() { return _unmanagedSpanner; }
    void addUnmanagedSpanner(Spanner*);
    void removeUnmanagedSpanner(Spanner*);

    Hairpin* addHairpin(HairpinType, const Fraction& tickStart, const Fraction& tickEnd, track_idx_t track);
    Hairpin* addHairpin(HairpinType, ChordRest* cr1, ChordRest* cr2 = nullptr);

    ChordRest* findCR(Fraction tick, track_idx_t track) const;
    ChordRest* findCRinStaff(const Fraction& tick, staff_idx_t staffIdx) const;
    void insertTime(const Fraction& tickPos, const Fraction& tickLen);

    std::shared_ptr<IEngravingFont> engravingFont() const { return m_engravingFont; }
    void setEngravingFont(std::shared_ptr<IEngravingFont> f) { m_engravingFont = f; }

    std::list<staff_idx_t> uniqueStaves() const;

    void transpositionChanged(Part* part, Interval oldTransposition, Fraction tickStart = { 0, 1 }, Fraction tickEnd = { -1, 1 });
    void transpositionChanged(Part* part, const Fraction& instrumentTick, Interval oldTransposition);

    void moveUp(ChordRest*);
    void moveDown(ChordRest*);
    EngravingItem* upAlt(EngravingItem*);
    Note* upAltCtrl(Note*) const;
    EngravingItem* downAlt(EngravingItem*);
    Note* downAltCtrl(Note*) const;

    EngravingItem* firstElement(bool frame = true);
    EngravingItem* lastElement(bool frame = true);

    size_t nmeasures() const;
    bool hasLyrics();
    bool hasHarmonies();
    int  lyricCount();
    int  harmonyCount();
    String extractLyrics();
    int keysig();
    int duration();
    int durationWithoutRepeats();

    void cmdInsertClef(Clef* clef, ChordRest* cr);

    void cmdExplode();
    void cmdImplode();
    void cmdSlashFill();
    void cmdSlashRhythm();
    void cmdResequenceRehearsalMarks();
    void cmdExchangeVoice(int, int);
    void cmdRemoveEmptyTrailingMeasures();
    void cmdRealizeChordSymbols(bool lit = true, Voicing v = Voicing(-1), HDuration durationType = HDuration(-1));

    Measure* firstTrailingMeasure(ChordRest** cr = nullptr);
    ChordRest* cmdTopStaff(ChordRest* cr = nullptr);

    std::shared_ptr<mu::draw::Pixmap> createThumbnail();
    String createRehearsalMarkText(RehearsalMark* current) const;
    String nextRehearsalMarkText(RehearsalMark* previous, RehearsalMark* current) const;

    Ret sanityCheckLocal();

    bool checkKeys();

    void switchToPageMode();

    PropertyValue getProperty(Pid) const override;
    bool setProperty(Pid, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

//    virtual QQueue<MidiInputEvent>* midiInputQueue();
    virtual std::list<MidiInputEvent>& activeMidiPitches();

    /// For MasterScores: returns the filename without extension
    /// For Scores: returns the excerpt name
    virtual String name() const;

    void cmdTimeDelete();
    bool checkTimeDelete(Segment* startSegment, Segment* endSegment);
    void doTimeDelete(Segment* startSegment, Segment* endSegment);
    void doTimeDeleteForMeasure(Measure*, Segment*, const Fraction&);

    Text* headerText(int index) const { return _headersText[index]; }
    Text* footerText(int index) const { return _footersText[index]; }
    void setHeaderText(Text* t, int index) { _headersText.at(index) = t; }
    void setFooterText(Text* t, int index) { _footersText.at(index) = t; }

    void cmdAddPitch(int note, bool addFlag, bool insert);
    void cmdToggleVisible();
    void forAllLyrics(std::function<void(Lyrics*)> f);

    void createPaddingTable();
    const PaddingTable& paddingTable() const { return _paddingTable; }
    double minimumPaddingUnit() const { return _minimumPaddingUnit; }

    void autoUpdateSpatium();

    friend class Chord;
};

static inline Score* toScore(EngravingObject* e)
{
    assert(!e || e->isScore());
    return static_cast<Score*>(e);
}

static inline const Score* toScore(const EngravingObject* e)
{
    assert(!e || e->isScore());
    return static_cast<const Score*>(e);
}

//---------------------------------------------------------
//   ScoreLoad
//---------------------------------------------------------

class ScoreLoad
{
    static int _loading;

public:
    ScoreLoad() { ++_loading; }
    ~ScoreLoad() { --_loading; }
    static bool loading() { return _loading > 0; }
};

DECLARE_OPERATORS_FOR_FLAGS(LayoutFlags)
} // namespace mu::engraving

#endif
