#!/usr/bin/ruby
def parse_size(size)
    match = size.match /(?<quantity>\d+)(?<unit>[GMKgmk]?)/
    fail("#{size} is not a valid size") unless match 
    match[:quantity].to_i * (case match[:unit].upcase
                                when "G" then 10**9
                                when "M" then 10**6
                                when "K" then 10**3
                                else 1
                             end)
end

puts "Usage: file-gen <file_name> <file_size> <piece_size>" if ARGV.length != 3

file_name = ARGV[0]
file_size = parse_size(ARGV[1])
piece_size = parse_size(ARGV[2])
n_pieces = (file_size / piece_size.to_f).ceil

fail "too many pieces" if n_pieces.to_s.length > 9

File.open(file_name, "w") do |file|
    (n_pieces).times do |i|
        str = i.to_s
        str << '.' * (piece_size - i.to_s.length)
        file << str[0...file_size]
        file_size -= piece_size
    end
end
