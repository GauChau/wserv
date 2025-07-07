#!/usr/bin/env ruby

require 'cgi'
require 'etc'

cgi = CGI.new
method = ENV['REQUEST_METHOD'] || ''
body = ''

# sys info
ruby_version = RUBY_VERSION
os_name = RUBY_PLATFORM
cwd = Dir.pwd
user = Etc.getlogin || ENV['USER'] || ENV['USERNAME'] || 'Unknown'

puts "Content-Type: text/plain\r\n"

puts "✅ CGI script executed successfully!\n\n"
puts "👤 User: #{user}"
puts "💎 Ruby version: #{ruby_version}"
puts "🖥 Platform: #{os_name}"
puts "📂 Current working directory: #{cwd}"
puts "🧾 Method: #{method}"
