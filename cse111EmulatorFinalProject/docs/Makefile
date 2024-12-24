
final_report.pdf: final_report.md
	pandoc --from=markdown \
	 --to=latex \
	 --toc \
	 -o $@ \
	 $< \
	 --shift-heading-level-by -1 \
	 --number-sections
