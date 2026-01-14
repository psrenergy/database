using Coverage

# Process coverage files from src directory
src_dir = joinpath(@__DIR__, "..", "src")
coverage = process_folder(src_dir)

# Create coverage directory
coverage_dir = joinpath(@__DIR__, "..", "coverage")
mkpath(coverage_dir)

# Generate LCOV report
LCOV.writefile(joinpath(coverage_dir, "lcov.info"), coverage)

# Print summary
covered, total = get_summary(coverage)
println("Coverage: $(covered)/$(total) lines ($(round(100*covered/total, digits=1))%)")
