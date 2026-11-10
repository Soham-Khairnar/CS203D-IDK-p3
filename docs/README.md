# Presentation handoff

For the full project and application explanation, see
[the application guide](../output/pdf/Project_3_Application_Guide.pdf).
Editable diagrams: [architecture](architecture.mmd) and
[command lifecycle](command_flow.mmd). Regenerate the guide with
`python docs/generate_project_guide.py` (documentation dependency: `reportlab`).

TODO (team): create docs/slides.pdf before the presentation. The brief allows at
most 10 slides, approximately 10 minutes plus 5 minutes for questions. Every member
presents their own contribution and analysis question.

Suggested outline:

1. Sensor-stream problem and operations.
2. Common CLI/interface and mode restrictions (Abhishek).
3. Segment-tree diagram, lazy mode and four-value mode (Soham).
4. Block-decomposition diagram and pending addition (Smita).
5. Oracle, differential tests and invariants (Test owner).
6. Main timing figure, block sizes and memory (Measurement owner).
7. Application/Q3: two partial-overlap boundaries, at most four visits per level.
8. Conclusion and live demo.

Include the other owners' analysis answers within their slides. Run the live demo
from the final tag with n = 1,000,000 after both production backends are ready.
This outline is a scaffold, not the required finished slides.pdf.
