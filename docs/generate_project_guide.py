"""Generate the explanation PDF. Documentation dependency: reportlab.

Usage: python docs/generate_project_guide.py
Diagrams use the same nodes/edges as the adjacent .mmd files, with a fixed vector
layout for print. Mermaid source is embedded in the guide and supplied separately.
"""
from pathlib import Path
from html import escape
from reportlab.lib import colors
from reportlab.lib.enums import TA_CENTER
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.lib.pagesizes import A4
from reportlab.platypus import (
    SimpleDocTemplate, Paragraph, Spacer, PageBreak, Table, TableStyle, Preformatted,
    Flowable, Image,
)

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'output' / 'pdf' / 'Project_3_Application_Guide.pdf'
NAVY = colors.HexColor('#16324f')
TEAL = colors.HexColor('#087e8b')
PALE = colors.HexColor('#edf5f8')
GRAY = colors.HexColor('#536273')
styles = getSampleStyleSheet()
styles.add(ParagraphStyle(name='TitleGuide', fontName='Helvetica-Bold', fontSize=29,
                         leading=34, textColor=NAVY, spaceAfter=18))
styles.add(ParagraphStyle(name='KickerGuide', fontSize=10, leading=14, textColor=TEAL,
                         spaceAfter=12))
styles.add(ParagraphStyle(name='BodyGuide', fontName='Helvetica', fontSize=10.2,
                         leading=15, textColor=NAVY, spaceAfter=9))
styles.add(ParagraphStyle(name='SmallGuide', fontSize=8.5, leading=12,
                         textColor=GRAY, spaceAfter=7))
styles.add(ParagraphStyle(name='HeadingGuide', fontName='Helvetica-Bold', fontSize=15,
                         leading=19, textColor=TEAL, spaceBefore=13, spaceAfter=9))
styles.add(ParagraphStyle(name='CodeGuide', fontName='Courier', fontSize=8.2,
                         leading=12, textColor=NAVY, backColor=PALE,
                         borderPadding=9, spaceBefore=5, spaceAfter=12))
styles.add(ParagraphStyle(name='CellGuide', fontSize=9, leading=13, textColor=NAVY))
styles.add(ParagraphStyle(name='CellHeadGuide', fontName='Helvetica-Bold', fontSize=9,
                         leading=13, textColor=colors.white))
story = []


def p(text, style='BodyGuide'):
    story.append(Paragraph(text, styles[style]))


def heading(text):
    p(text, 'HeadingGuide')


def code(text):
    story.append(Preformatted(text.strip('\n'), styles['CodeGuide']))


def table(rows, widths):
    data = [[Paragraph(escape(str(cell)), styles['CellHeadGuide' if i == 0 else 'CellGuide'])
             for cell in row] for i, row in enumerate(rows)]
    t = Table(data, colWidths=widths, hAlign='LEFT', repeatRows=1)
    t.setStyle(TableStyle([
        ('BACKGROUND', (0, 0), (-1, 0), NAVY),
        ('ROWBACKGROUNDS', (0, 1), (-1, -1), [colors.white, PALE]),
        ('VALIGN', (0, 0), (-1, -1), 'TOP'),
        ('LEFTPADDING', (0, 0), (-1, -1), 9),
        ('RIGHTPADDING', (0, 0), (-1, -1), 9),
        ('TOPPADDING', (0, 0), (-1, -1), 8),
        ('BOTTOMPADDING', (0, 0), (-1, -1), 8),
        ('LINEBELOW', (0, 0), (-1, 0), 1, TEAL),
    ]))
    story.extend([t, Spacer(1, 10)])


def page(number, title, subtitle):
    if story:
        story.append(PageBreak())
    p(f'PROJECT 3 / {number:02d}', 'KickerGuide')
    p(title, 'TitleGuide')
    p(subtitle, 'SmallGuide')


class Diagram(Flowable):
    """Vector print layout of the Mermaid graph, with numbered companion source."""
    def __init__(self, kind):
        super().__init__()
        self.width = 499
        self.height = 385 if kind == 'architecture' else 445
        self.kind = kind

    def draw(self):
        c = self.canv
        def box(x, y, w, h, lines, fill=PALE):
            c.setFillColor(fill)
            c.setStrokeColor(TEAL)
            c.roundRect(x, y, w, h, 7, stroke=1, fill=1)
            c.setFillColor(NAVY)
            c.setFont('Helvetica', 9)
            start = y + h / 2 + (len(lines)-1)*6 - 3
            for j, line in enumerate(lines):
                c.drawCentredString(x+w/2, start-j*12, line)
        def arrow(points, label=None, at=None):
            c.setStrokeColor(GRAY)
            c.setLineWidth(1)
            path = c.beginPath()
            path.moveTo(*points[0])
            for point in points[1:]:
                path.lineTo(*point)
            c.drawPath(path)
            x, y = points[-1]
            px, py = points[-2]
            import math
            angle = math.atan2(y-py, x-px)
            for delta in (-.5, .5):
                c.line(x, y, x-6*math.cos(angle+delta), y-6*math.sin(angle+delta))
            if label:
                c.setFont('Helvetica', 8)
                c.setFillColor(GRAY)
                c.drawString(*(at or points[0]), label)

        if self.kind == 'architecture':
            box(144, 346, 210, 34, ['Operation file / standard input'])
            box(144, 294, 210, 34, ['run.py: optional Python launcher'])
            box(144, 242, 210, 34, ['main.cpp: select A or B'])
            box(144, 190, 210, 34, ['runner.cpp: validate and replay'])
            box(144, 138, 210, 34, ['makeBackend: select backend'])
            for top, bottom in [(346,328),(294,276),(242,224),(190,172)]:
                arrow([(249,top),(249,bottom)])
            for x, lines, fill in [(0,['A + LAZY','implemented'],colors.HexColor('#e6f4ed')),
                                   (174,['A + STRETCH','implemented'],colors.HexColor('#e6f4ed')),
                                   (348,['B + LAZY','implemented'],colors.HexColor('#e6f4ed'))]:
                box(x, 63, 150, 48, lines, fill)
                arrow([(249,138),(249,125),(x+75,125),(x+75,111)])
                arrow([(x+75,63),(x+75,48),(249,48),(249,37)])
            box(129, 3, 240, 34, ['RangeAnalytics: query results to stdout'])
            arrow([(354,207),(487,207),(487,20),(369,20)])
        else:
            box(130,405,230,34,['Read mode and n; validate header'])
            box(130,348,230,34,['Read exactly n initial values'])
            box(130,291,230,34,['Construct selected backend'])
            box(130,234,230,34,['Read next command'])
            box(130,177,230,34,['Validate syntax, mode and indices'])
            box(130,120,230,34,['Dispatch to interface method'])
            box(130,63,230,34,['Query: print one answer','Update: print nothing'])
            for top,bottom in [(405,382),(348,325),(291,268),(234,211),(177,154),(120,97)]:
                arrow([(245,top),(245,bottom)])
            box(386,234,106,34,['EOF: exit 0'])
            arrow([(360,251),(386,251)])
            box(0,177,102,34,['Error: stderr','Exit 1'])
            arrow([(130,194),(102,194)])
            arrow([(130,422),(50,422),(50,211)],'invalid',(55,398))
            arrow([(245,63),(245,38),(375,38),(375,251),(360,251)],'next',(380,116))
            arrow([(130,251),(114,251),(114,280),(245,280),(245,268)],'blank',(74,273))


page(1, 'Range Analytics\non Sensor Streams',
     'CS 203D - Project 3 | Team IDK | Application guide for Abhishek Verma | 20 November 2026')
p('A guide to the whole project and the application code built for your role. '
  'Use it to understand the repository, coordinate integration, and prepare your own explanation.')
heading('The problem in one paragraph')
p('An array holds up to one million sensor readings. Operators correct individual '
  'readings, add calibration offsets to ranges, and ask for sums, minima, or the '
  'highest-total contiguous stretch. The project compares two hand-built data '
  'structures behind the same command-line application.')
heading('What is complete today?')
table([['Area','Status'],
       ['Your application','CLI, parser, validation, shared interface, factory, launcher, examples and application tests implemented.'],
       ['Structure A','LAZY and STRETCH trees connect to the app through the SegmentTree adapter; Soham’s Q1 is documented.'],
       ['Structure B','LAZY block decomposition is implemented with unit-test source and invariant inspection.'],
       ['Testing / measurements','Full suite passes with both structures: 54 differential configurations and 2,784,000 comparisons. Benchmark data/figures are present.']], [115,384])
heading('How to read this guide')
p('Pages 2-3 explain the project and architecture. Pages 4-7 explain your code '
  'and trace commands. Page 8 explains verification and current benchmark results. '
  'Page 9 develops Q3. Pages 10-11 contain Mermaid source, integration steps and viva preparation.')
p('<b>Scope:</b> this is an explanatory guide, not the team\'s finished submission '
  'report. Performance values are read from the repository\'s checked-in measurement '
  'summary and are identified as such.', 'SmallGuide')

page(2, 'The whole project', 'Two modes, two structures, one operation-file format.')
table([['Command','Meaning','LAZY A/B','STRETCH A'],
       ['U i v','Set a[i] to v','Yes','Yes'],
       ['A l r x','Add x to a[l..r]','Yes','No'],
       ['S l r','Print range sum','Yes','Yes'],
       ['M l r','Print range minimum','Yes','No'],
       ['B l r','Print best nonempty stretch sum','No','Yes']], [75,244,90,90])
heading('Structure A: segment tree - Soham')
p('A balanced binary tree stores summaries of contiguous intervals. LAZY mode '
  'keeps sum, minimum and a pending addition. Adding x to a fully covered node '
  'of length k changes sum by k*x, minimum by x and its pending tag by x. '
  'The tag is pushed before descending when necessary.')
p('STRETCH mode keeps total, best prefix, best suffix and best stretch. Leaves '
  'start with all four values equal to their reading, so empty stretches never '
  'win over negative readings. For left summary L and right summary R:')
code('total  = L.total + R.total\nprefix = max(L.prefix, L.total + R.prefix)\nsuffix = max(R.suffix, R.total + L.suffix)\nbest   = max(L.best, R.best, L.suffix + R.prefix)')
heading('Structure B: block decomposition - Smita')
p('Split readings into blocks of about sqrt(n) elements. Each block stores its '
  'sum, minimum and pending addition. Full blocks use the summary; the two end '
  'fragments are handled element by element. This backend supports LAZY only.')
p('<b>Complexity:</b> implemented segment-tree operations take O(log n); default '
  'block decomposition takes O(sqrt(n)) per operation. Both use O(n) storage.')
p('Range addition and best stretch are intentionally not combined: a constant '
  'offset changes each candidate stretch by its length times that offset.')

page(3, 'Architecture and ownership', 'Figure 1. Current project architecture; editable source: docs/architecture.mmd.')
story.append(Diagram('architecture'))
heading('Your boundary of responsibility')
p('You own selection, parsing, validation, dispatch and output. The structure '
  'owners supply the algorithms invoked by the interface. The Test owner supplies '
  'the full oracle, differential tests and invariants; the Measurement owner '
  'collects timings, memory data and figures.')
p('The diagram uses a fixed vector layout of the accompanying Mermaid graph. '
  'The runner invokes methods through RangeAnalytics after makeBackend creates '
  'the selected concrete object. The interface is a contract, not an extra '
  'data-storage algorithm.')
p('<b>CLI versus React:</b> the required application reads standard input and writes '
  'standard output, allowing examiner files to run unchanged. A React dashboard '
  'could later call a backend, but it is not implemented or required for this CLI.')

page(4, 'Your part: the file map', 'Application owner: Abhishek Verma (250051002), analysis question Q3.')
table([['File','What it does'],
       ['build.py','Compiles the C++ application with C++17, optimization and warning flags. Outputs build/run.exe on Windows.'],
       ['run.py','Optional portable launcher. Parses --structure and --input, opens the file, then runs the compiled C++ program.'],
       ['app/main.cpp','Validates A/B selection, handles --help, configures I/O, calls run(), catches errors and sets exit codes.'],
       ['app/runner.hpp','Declares run() and BackendFactory so tests can inject a controlled backend.'],
       ['app/runner.cpp','Reads the header and initial values, validates commands, calls methods and prints answers.'],
       ['src/range_analytics.hpp','Defines Value, Index, Mode, Structure and the abstract operation contract.'],
       ['src/backend_factory.cpp','Constructs SegmentTree for A; BlockDecomposition for B/LAZY. Rejects B/STRETCH.'],
       ['src/structB/block_decomposition.cpp','Implemented LAZY backend with raw readings, per-block sum/minimum/pending add, range operations and selfCheck.'],
       ['tests/ and bench/','Oracle, differential/invariant framework, workloads, benchmark runners and plotting scripts.']], [156,343])
heading('Why is there Python in a C++ project?')
p('Python only builds or launches the compiled program and orchestrates tests. '
  'It does not perform production range calculations. run.py is useful on '
  'PowerShell because --input avoids shell-specific input-redirection syntax. '
  'The C++ executable can also be invoked directly by the examiner.')
heading('Why keep main.cpp small?')
p('main.cpp handles the process boundary. run() takes input and output streams, '
  'so tests can pass string streams without launching a subprocess for every '
  'case. This also keeps command parsing independent of either data structure.')

page(5, 'How a command travels', 'Figure 2. Command lifecycle; editable source: docs/command_flow.mmd.')
story.append(Diagram('commands'))
p('The diagram shows the successful path after backend construction. Currently '
  'the production constructors throw, so valid production inputs stop there '
  'until your teammates complete them. Backend exceptions are caught by main.cpp.', 'SmallGuide')
heading('Validation happens before dispatch')
p('number() reads a signed 64-bit integer. index() checks the 1..n bounds. '
  'endOfLine() rejects extra tokens. The runner checks command legality and '
  'left &lt;= right before calling the backend. A malformed command therefore '
  'does not partially apply an update.')
p('Blank command lines are ignored. Commands stream through EOF instead of '
  'being stored together. The parser does retain the initial vector and the '
  'current text line; this is not a constant-memory application.')

page(6, 'The shared C++ contract', 'Agree on these semantics before integrating your teammates\' code.')
code('using Value = std::int64_t;\nusing Index = std::size_t;\n\nvirtual void pointSet(Index i, Value value) = 0;\nvirtual void rangeAdd(Index l, Index r, Value delta) = 0;\nvirtual Value rangeSum(Index l, Index r) = 0;\nvirtual Value rangeMin(Index l, Index r) = 0;\nvirtual Value bestStretch(Index l, Index r) = 0;')
heading('Indices and ownership')
p('Public operations use 1-based inclusive indices, matching the assignment. '
  'The constructor receives a normal C++ vector whose first element is at '
  'offset zero. A backend may use its preferred internal indexing, but it '
  'must translate at its own public boundary consistently.')
p('The factory returns unique_ptr&lt;RangeAnalytics&gt;. This owns the concrete '
  'backend and releases it automatically. The virtual destructor ensures the '
  'derived backend is destroyed correctly through the base pointer. Virtual '
  'methods let the runner call a single interface for either implementation.')
heading('Assignment is different from addition')
p('After adding 10 to a range, U 1 -5 means the actual value of reading 1 '
  'becomes -5. It does not add -5, and must not leave an old pending offset '
  'applied on top of -5. Both owners need tests for this interaction.')
heading('Supported operations and arithmetic')
p('The runner rejects A and M in STRETCH, B in LAZY, and all B/STRETCH files. '
  'Backends should also reject unsupported methods if called directly. '
  'Use signed 64-bit arithmetic throughout. Inputs must keep intermediate '
  'and final values in range; arithmetic overflow detection is not implemented.')
heading('The test seam')
p('BackendFactory is a callable passed to run(). Production defaults to '
  'makeBackend. Application tests instead return RecordingArray, a small '
  'test-only implementation. This verifies your control flow before the real '
  'algorithms exist. It is not a production fallback or a replacement for '
  'the Test owner\'s independent oracle and differential suite.')

page(7, 'Trace the input by hand', 'These are intended semantics, checked with the application test backend.')
code('LAZY 5\n1 2 3 4 5\nS 1 5\nA 2 4 10\nM 1 5\nU 1 -5\nS 1 3')
table([['Command','Array afterwards','Output'],
       ['Initial','[1, 2, 3, 4, 5]',''],
       ['S 1 5','[1, 2, 3, 4, 5]','15'],
       ['A 2 4 10','[1, 12, 13, 14, 5]','None'],
       ['M 1 5','[1, 12, 13, 14, 5]','1'],
       ['U 1 -5','[-5, 12, 13, 14, 5]','None'],
       ['S 1 3','[-5, 12, 13, 14, 5]','20']], [92,321,86])
heading('STRETCH example')
p('For [-4, 3, -1, 5, -8], B 1 5 returns 7 from [3, -1, 5]. S 1 5 returns '
  '-5. After U 2 -6, the first three readings are [-4, -6, -1], so B 1 3 '
  'returns -1. It must not return zero, because an empty stretch is forbidden.')
heading('Useful failure examples')
p('STRETCH with --structure B is rejected on line 1. S 2 1 is a reversed '
  'range; U 0 8 has an invalid index; S 1 3 extra has an extra token. '
  'B in LAZY is an unsupported operation. All are rejected before dispatch.')
p('Errors go to stderr and processing stops. Successful answers printed before '
  'a later error remain in stdout. Update commands print no success messages.')

page(8, 'Build, run and verify', 'Commands run from the repository root. Both structures are integrated and the suite passes.')
code('python build.py\npython tests/run_tests.py\npython bench/benchmark.py --quick\npython bench/benchmark.py')
p('Structure A now has LAZY and STRETCH implementations behind a SegmentTree '
  'adapter to the app interface. Structure B supplies LAZY block decomposition. '
  'The full build and test suite completed successfully in this review. The sample '
  'LAZY operation file returns 15, 1 and 20.')
heading('Testing and measurement code now present')
p('The Test owner has added a plain-array/Kadane oracle, seven workload profiles, '
  'a differential runner with base seed 203003, invariant checks every 500 '
  'operations, and worst-case input generation/modeling. The documented full '
  'suite uses 100,000 operations per configuration. All 54 configurations passed '
  'here: 2,784,000 backend-operation comparisons and 5,568 invariant checks.')
p('The benchmark script uses C++ runners for A lazy, A stretch and B lazy. '
  'Recorded settings are n = 10k, 30k, 100k, 300k and 1m; 200,000 operations; '
  'five repetitions; seeds 42 and 97; short/long/mixed profiles; and six block '
  'sizes. The summary reports 10 trials per row, from five repetitions over two seeds.')
heading('A sample measured comparison')
figure_path = ROOT / 'results' / 'measurement' / 'figures' / 'lazy_vs_n_mixed.png'
if figure_path.exists():
    story.append(Image(str(figure_path), width=360, height=225))
    p('Figure 3. Mixed LAZY workload, mean nanoseconds per operation. The checked-in '
      'summary at n=1,000,000 reports about 918 ns/op for A and 2,846 ns/op for B. '
      'At n=10,000 it reports about 369 and 477 ns/op. These are recorded results '
      'from the repository summary, not a fresh run in this review. Machine: Windows '
      '11, MinGW GCC 6.3.0. Interpret them with the raw CSV and benchmark method.', 'SmallGuide')
else:
    p('The benchmark figure lazy_vs_n_mixed.png was not found; regenerate figures from the raw CSV.', 'SmallGuide')
p('Reading the figure: A is faster; B uses less storage. This agrees with the '
  'expected complexity trend. Values are checked-in summaries, not a fresh run.', 'SmallGuide')

page(9, 'Your Q3 explanation', 'Why does a segment-tree range query visit O(log n) nodes?')
p('A balanced tree has height ceil(log2(n)), but that alone does not prove a '
  'logarithmic query: the whole tree still has O(n) nodes. The crucial rule is '
  'to recurse only when a node interval partially overlaps the query.')
heading('Three outcomes for every visited node')
p('<b>Disjoint:</b> return immediately. <b>Fully covered:</b> return its stored '
  'summary immediately. <b>Partial overlap:</b> visit children and combine results.')
p('At a fixed depth, node intervals are disjoint and ordered. A contiguous '
  'query can partially overlap only the intervals containing its two boundaries. '
  'There are therefore at most two partial nodes per level. Each has at most '
  'two children, giving at most four visited nodes at the next level, including '
  'disjoint calls that immediately stop.')
code('height h = ceil(log2(n))\nvisited nodes <= 1 + 4*h\nconstant work per visit => O(log n) query time')
heading('Example: n = 16, query [3,14]')
table([['Depth','Visited intervals','Recurse from'],
       ['0','[1,16]','[1,16]'],
       ['1','[1,8], [9,16]','Both'],
       ['2','[1,4], [5,8], [9,12], [13,16]','[1,4], [13,16]'],
       ['3','[1,2], [3,4], [13,14], [15,16]','None']], [45,292,162])
p('The middle nodes at depth 2 return stored answers. At depth 3, [3,4] and '
  '[13,14] are covered while [1,2] and [15,16] are disjoint. Total: 11 visits, '
  'compared with 31 nodes in the entire tree.')
p('Sum/minimum merges cost O(1). A four-value STRETCH merge also costs O(1) '
  'and must preserve left-to-right order. Pushing one lazy tag to two children '
  'is constant work. For n = 1, write O(1 + log n) to include the one-node case.')

page(10, 'Editable Mermaid diagrams', 'The print figures on pages 3 and 5 use these graph relationships.')
p('Open either .mmd file in a Mermaid-capable editor, or paste the source into '
  'a Mermaid code block in Markdown. The PDF contains vector layouts for print; '
  'an editor may arrange the same graph differently.')
heading('Figure 1 source: architecture.mmd')
code((ROOT / 'docs' / 'architecture.mmd').read_text(encoding='utf-8'))
heading('Figure 2 source: command_flow.mmd')
code((ROOT / 'docs' / 'command_flow.mmd').read_text(encoding='utf-8'))

page(11, 'Integration and viva preparation', 'Use this as a handoff and explanation checklist.')
heading('What to do next with your team')
p('<b>1. Agree on the interface.</b> Confirm 1-based indices, inclusive ranges, '
  'absolute assignment, nonempty stretches and 64-bit values. Keep method '
  'signatures compatible with the factory and runner.')
p('<b>2. Implement the structures.</b> Soham completes SegmentTree for both '
  'modes; Smita completes BlockDecomposition for LAZY. Replace the explicit '
  'placeholder constructors and methods, without printing from backends.')
p('<b>3. Integrate and test.</b> Run sample files through the production CLI, '
  'then the Test owner\'s differential and invariant suites. Compare A/B outputs '
  'on identical LAZY files. Measure only completed, verified algorithms.')
p('<b>4. Finish submission artifacts.</b> Fill the two unnamed roles, collect '
  'results, finish each report section, export REPORT.pdf and create slides. '
  'Keep the required commit history and final-tag workflow from the assignment.')
heading('Be ready to explain these choices')
table([['Viva prompt','Your explanation'],
       ['Why a common interface?','The parser uses one operation contract while the backend changes.'],
       ['Why inject a factory?','It makes application control flow testable before real structures exist.'],
       ['Why stderr?','Diagnostics must not contaminate examiner query output.'],
       ['Why not zero for negative stretches?','The problem requires a nonempty contiguous stretch.'],
       ['What did Codex do?','Generated the app adapter and Q1 draft; review and understand them before submission.']], [155,344])
heading('Sources and reproducibility')
p('This guide is based on CS203-D_Mini_Projects_4-1.pdf (Project 3 on pages 6-7; '
  'common requirements on pages 1-4 and 9-10), README.md, REPORT.md, TOOLS.md, '
  'app/, src/, build.py, run.py and tests/. It describes the repository state '
  'reviewed on 20 November 2026.', 'SmallGuide')
p('Regenerate with: python docs/generate_project_guide.py (requires reportlab). '
  'This is a documentation dependency only. Editable diagram sources are '
  'docs/architecture.mmd and docs/command_flow.mmd. Codex assistance is declared '
  'in TOOLS.md; review and understand the assisted code before submission.', 'SmallGuide')


def footer(canvas, doc):
    w, h = A4
    canvas.setStrokeColor(TEAL)
    canvas.setLineWidth(0.6)
    canvas.line(48, h-35, w-48, h-35)
    canvas.setFont('Helvetica', 8)
    canvas.setFillColor(GRAY)
    canvas.drawString(48, 27, 'TEAM IDK  |  PROJECT 3  |  APPLICATION GUIDE')
    canvas.drawRightString(w-48, 27, str(doc.page))


if __name__ == '__main__':
    OUT.parent.mkdir(parents=True, exist_ok=True)
    doc = SimpleDocTemplate(str(OUT), pagesize=A4, rightMargin=48, leftMargin=48,
                            topMargin=51, bottomMargin=47,
                            title='Project 3 - Whole Project and Application Guide',
                            author='Abhishek Verma | Team IDK | Codex-assisted')
    doc.build(story, onFirstPage=footer, onLaterPages=footer)
    print(OUT)
