local Ball = {}
Ball.__index = Ball

-- Every hit speeds the ball up, so rallies get harder the longer they last. The cap keeps the
-- ball from moving far enough in one frame to pass through a paddle.
local SPEED_INCREASE = 1.1
local MAX_SPEED_MULTIPLIER = 3.0

function Ball.new()
    local self = setmetatable({}, Ball)

    self.radius = 16.0
    self.velocity_x = 600.0
    self.velocity_y = 400.0
    self:reset()

    return self
end

function Ball:reset()
    self.x = rivet.graphics.get_width() / 2.0
    self.y = rivet.graphics.get_height() / 2.0

    -- Each serve starts back at the base speed.
    self.speed_multiplier = 1.0
end

function Ball:update(delta)
    local width = rivet.graphics.get_width()

    self.x = self.x + self.velocity_x * self.speed_multiplier * delta
    self.y = self.y + self.velocity_y * self.speed_multiplier * delta

    -- The ball only bounces off the side walls. Leaving through the top or bottom is a point,
    -- which main.lua scores.
    if self.x - self.radius < 0.0 then
        self.x = self.radius
        self.velocity_x = -self.velocity_x
    elseif self.x + self.radius > width then
        self.x = width - self.radius
        self.velocity_x = -self.velocity_x
    end
end

-- Pushes the ball out of the side of the paddle it hit and sends it away from that side, so it
-- can't collide again on the next frame and get stuck inside.
function Ball:bounce_off(paddle)
    local closest_x = math.max(paddle.x, math.min(self.x, paddle.x + paddle.width))
    local closest_y = math.max(paddle.y, math.min(self.y, paddle.y + paddle.height))
    local distance_x = self.x - closest_x
    local distance_y = self.y - closest_y

    if distance_x * distance_x + distance_y * distance_y > self.radius * self.radius then
        return
    end

    if self.y < paddle.y + paddle.height / 2.0 then
        self.y = paddle.y - self.radius
        self.velocity_y = -math.abs(self.velocity_y)
    else
        self.y = paddle.y + paddle.height + self.radius
        self.velocity_y = math.abs(self.velocity_y)
    end

    self.speed_multiplier = math.min(self.speed_multiplier * SPEED_INCREASE, MAX_SPEED_MULTIPLIER)
end

function Ball:draw()
    -- A 2px ring rather than a 1px circle outline, so the outline survives the viewport being
    -- scaled down to the window.
    local thickness = 2.0

    rivet.graphics.set_color(1.0, 1.0, 1.0)
    rivet.graphics.ring(self.x, self.y, self.radius - thickness, self.radius)
end

return Ball
